#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Minimal type re-declarations matching the libpng code fragment */
typedef unsigned char png_byte;
typedef struct {
    size_t rowbytes;
} png_row_info;

/* Very small vector-like type to mimic MSA 16-byte vectors */
typedef struct { png_byte b[16]; } v16u8;

/* Stub implementations for the MSA load/store intrinsics/macros */
static inline v16u8 LD_UB(const png_byte *p) {
    v16u8 v;
    /* Safe when caller guarantees >=16 bytes; our test will ensure this for prev_row */
    memcpy(v.b, p, 16);
    return v;
}

static inline void ST_UB(v16u8 v, png_byte *p) {
    /* Always writes 16 bytes, which is the root cause of the overflow in the tail case */
    memcpy(p, v.b, 16);
}

static inline v16u8 add_v16u8(v16u8 a, v16u8 b) {
    v16u8 r;
    for (int i = 0; i < 16; ++i) r.b[i] = (png_byte)(a.b[i] + b.b[i]);
    return r;
}

/* Vulnerable function reimplemented to preserve the buggy tail behavior: for a
 * remaining length of 1..15 bytes, it still uses ST_UB to store 16 bytes. */
__attribute__((noinline))
static void png_read_filter_row_up_msa(png_row_info *row_info, png_byte *row,
                                       const png_byte *prev_row)
{
    size_t istop = row_info->rowbytes;
    png_byte *rp = row;
    const png_byte *pp = prev_row;

    /* Emulate only the problematic tail case: when only a remainder < 16 bytes
     * remains, the code still does a 16-byte vector store. */
    if ((istop & 0xF) != 0) {
        /* In the original code it does: src0 = LD_UB(rp); src4 = LD_UB(pp); src0 += src4; ST_UB(src0, rp);
         * To avoid an earlier out-of-bounds READ from rp (since rp has < 16 bytes here),
         * we emulate src0 as zero and only load pp safely (we provide >=16 bytes there).
         * The bug is the 16-byte STORE to rp, which will overflow the row buffer. */
        v16u8 zero = { {0} };
        v16u8 src4 = LD_UB(pp);   /* safe: prev_row will be >= 16 bytes */
        v16u8 src0 = add_v16u8(zero, src4);
        ST_UB(src0, rp);          /* writes 16 bytes even if only 1..15 are valid -> OOB write */
    }
}

int main(void) {
    /* Choose a remainder length between 1 and 15 to trigger the buggy tail store */
    const size_t tail_len = 15; /* any value 1..15 works */

    /* Allocate exactly tail_len bytes for the row to make the 16-byte store overflow */
    png_byte *row = (png_byte *)malloc(tail_len);
    if (!row) {
        perror("malloc(row)");
        return 1;
    }
    memset(row, 0x11, tail_len);

    /* prev_row must be at least 16 bytes because the stub LD_UB reads 16 bytes */
    png_byte *prev_row = (png_byte *)malloc(16);
    if (!prev_row) {
        perror("malloc(prev_row)");
        return 1;
    }
    memset(prev_row, 0x22, 16);

    png_row_info info;
    info.rowbytes = tail_len;  /* only remainder present (1..15) */

    /* This call will perform a 16-byte store into a buffer of size tail_len, causing
     * an out-of-bounds WRITE that AddressSanitizer will report. */
    png_read_filter_row_up_msa(&info, row, prev_row);

    /* Prevent optimizing away */
    volatile png_byte sink = row[0];
    (void)sink;

    free(row);
    free(prev_row);
    return 0;
}
