#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>

/* Minimal types mirroring libpng usage */
typedef unsigned char png_byte;
typedef struct {
    size_t rowbytes;
} png_row_info;

/* Minimal vector type and helpers to emulate MSA intrinsics on any platform */
typedef struct { uint8_t b[16]; } v16u8;

static inline v16u8 vec_zero(void) {
    v16u8 v; memset(v.b, 0, sizeof(v.b)); return v;
}

static inline v16u8 add_vec(v16u8 a, v16u8 b) {
    v16u8 r; for (int i = 0; i < 16; i++) r.b[i] = (uint8_t)(a.b[i] + b.b[i]); return r;
}

static inline void store_vec(uint8_t *p, v16u8 v) {
    /* This is the critical write which will go out-of-bounds when the caller
       provides too small a row buffer but the code writes 32 bytes. */
    memcpy(p, v.b, 16);
}

/* Stubbed versions of MSA load/add/store macros used by the vulnerable code */
#define LD_UB(p)            vec_zero()                   /* ignore memory to avoid OOB reads */
#define LD_UB2(p, s, a, b)  do { (a) = vec_zero(); (b) = vec_zero(); } while (0)
#define ADD2(a0, b0, a1, b1, d0, d1) \
    do { (d0) = add_vec((a0), (b0)); (d1) = add_vec((a1), (b1)); } while (0)
#define ST_UB(v, p)         store_vec((p), (v))
#define ST_UB2(v0, v1, p, stride) \
    do { store_vec((p), (v0)); store_vec((p) + (stride), (v1)); } while (0)

/* Reimplementation of the vulnerable tail of png_read_filter_row_up_msa.
   We only model the specific path that exhibits the bug: cnt16 && cnt without cnt32. */
void png_read_filter_row_up_msa(png_row_info *row_info, png_byte *row, const png_byte *prev_row) {
    size_t istop = row_info->rowbytes;

    png_byte *rp = row;
    const png_byte *pp = prev_row;

    size_t cnt32 = istop & 0x20; /* has 32-byte chunk left? */
    size_t cnt16 = istop & 0x10; /* has 16-byte chunk left? */
    size_t cnt   = istop & 0x0F; /* leftover < 16 bytes */

    /* Only emulate the buggy case: when 17..31 bytes remain, i.e. cnt16 && cnt, but no cnt32 */
    if (!cnt32 && cnt16 && cnt) {
        v16u8 src0, src1, src4, src5;
        LD_UB2(rp, 16, src0, src1);
        LD_UB2(pp, 16, src4, src5);
        ADD2(src0, src4, src1, src5, src0, src1);
        /* BUG: writes 32 bytes unconditionally, even though only 17..31 bytes remain */
        ST_UB2(src0, src1, rp, 16);
        rp += 32;
    }
}

int main(void) {
    /* Craft rowbytes so that: (istop & 0x20) == 0, (istop & 0x10) != 0, (istop & 0x0F) != 0.
       For example, 17 (0x11) satisfies this and triggers the cnt16 && cnt path. */
    const size_t rowbytes = 17;   /* Remaining bytes = 17..31 triggers the bug */

    /* Allocate a row buffer with exactly 'rowbytes' bytes to make the 32-byte store overflow. */
    png_byte *row = (png_byte *)malloc(rowbytes);
    if (!row) { perror("malloc row"); return 1; }
    memset(row, 0xAA, rowbytes);

    /* prev_row is not actually read thanks to stubbed loads; allocate a small buffer anyway. */
    png_byte *prev_row = (png_byte *)malloc(32);
    if (!prev_row) { perror("malloc prev_row"); return 1; }
    memset(prev_row, 0x55, 32);

    png_row_info info;
    info.rowbytes = rowbytes;

    /* This call will perform a 32-byte store into a 17-byte buffer, causing ASan OOB write. */
    png_read_filter_row_up_msa(&info, row, prev_row);

    /* Touch memory to keep row live. */
    printf("row[0]=%u\n", (unsigned)row[0]);

    free(prev_row);
    free(row);
    return 0;
}