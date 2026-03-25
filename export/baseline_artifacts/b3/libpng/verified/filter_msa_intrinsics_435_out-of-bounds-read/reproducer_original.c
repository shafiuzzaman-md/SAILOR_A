#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* Minimal type re-declarations matching libpng usage */
typedef unsigned char png_byte;
typedef struct {
    size_t rowbytes;
} png_row_info;

/* Minimal MSA-like vector type and helpers to emulate 16-byte loads/stores */
typedef struct { unsigned char b[16]; } v16u8;

static inline v16u8 LD_UB(const void *p) {
    v16u8 r;
    /* Unconditionally read 16 bytes to emulate the buggy vector load. */
    memcpy(r.b, p, 16); /* ASan will catch OOB here when p has <16 valid bytes */
    return r;
}

static inline void ST_UB(v16u8 v, void *p) {
    memcpy(p, v.b, 16);
}

static inline v16u8 add_v16u8(v16u8 a, v16u8 b) {
    for (int i = 0; i < 16; i++) {
        a.b[i] = (unsigned char)(a.b[i] + b.b[i]);
    }
    return a;
}

/* Vulnerable function stub mirroring the problematic tail branch */
void png_read_filter_row_up_msa(png_row_info *row_info, png_byte *row,
                                const png_byte *prev_row)
{
    /* Emulate the tail handling logic where cnt16 = (len & 16) != 0,
       cnt = len & 15; for 1..15 bytes, cnt16==0 and cnt>0, so (cnt16||cnt) true. */
    size_t cnt16 = (row_info->rowbytes & 16) != 0;
    size_t cnt   = (row_info->rowbytes & 15);

    png_byte *rp = row;
    const png_byte *pp = prev_row;

    /* Only the tail branch that triggers the bug is modeled here. */
    if (cnt16 || cnt) {
        v16u8 src0 = LD_UB(rp);     /* Safe in our setup (row has >=16 bytes) */
        v16u8 src4 = LD_UB(pp);     /* BUG: OOB read when only 1..15 bytes remain */
        pp += 16;

        src0 = add_v16u8(src0, src4);
        ST_UB(src0, rp);            /* Keep this in-bounds to isolate the read bug */
        rp += 16;
    }
}

int main(void)
{
    /* Set rowbytes to 1 to force the 1..15 tail path: cnt16=0, cnt=1 */
    png_row_info info;
    info.rowbytes = 1; /* Remaining length in [1, 15] triggers LD_UB overread */

    /* Allocate row large enough so the 16-byte load/store on row is in-bounds */
    size_t row_sz = 32;
    png_byte *row = (png_byte *)malloc(row_sz);
    assert(row);
    memset(row, 0x11, row_sz);

    /* Allocate prev_row with only 1 valid byte so 16-byte LD_UB overreads */
    size_t prev_sz = 1; /* Deliberately too small */
    png_byte *prev_row = (png_byte *)malloc(prev_sz);
    assert(prev_row);
    memset(prev_row, 0x22, prev_sz);

    /* Call the vulnerable function; ASan should report OOB READ from prev_row */
    png_read_filter_row_up_msa(&info, row, prev_row);

    /* Use the result a bit to avoid any dead-code elimination (though -O0). */
    printf("row[0]=%u\n", (unsigned)row[0]);

    free(prev_row);
    free(row);
    return 0;
}
