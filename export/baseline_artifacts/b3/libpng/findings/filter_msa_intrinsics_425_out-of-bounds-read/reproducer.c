#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Minimal libpng-like typedefs */
typedef unsigned char png_byte;

typedef struct png_row_info_s {
    size_t rowbytes;
} png_row_info;

/* A simple 16-byte vector type to mimic v16u8 */
typedef struct { unsigned char v[16]; } v16u8;

/* Load 16 bytes from p into a v16u8. This will trigger ASan if p is OOB. */
static inline v16u8 LD_UB(const unsigned char *p) {
    v16u8 x;
    for (int i = 0; i < 16; i++) x.v[i] = p[i];
    return x;
}

/* Store 16 bytes to p from a v16u8. */
static inline void ST_UB(const v16u8 x, unsigned char *p) {
    for (int i = 0; i < 16; i++) p[i] = x.v[i];
}

/* Macros modeled after the MSA helpers used by libpng */
#define LD_UB2(p, stride, a, b) do { \
    (a) = LD_UB((const unsigned char*)(p)); \
    (b) = LD_UB((const unsigned char*)(p) + (stride)); \
} while (0)

#define ST_UB2(a, b, p, stride) do { \
    ST_UB((a), (unsigned char*)(p)); \
    ST_UB((b), (unsigned char*)(p) + (stride)); \
} while (0)

static inline v16u8 ADD2_impl(v16u8 a, v16u8 b) {
    v16u8 r;
    for (int i = 0; i < 16; i++) r.v[i] = (unsigned char)(a.v[i] + b.v[i]);
    return r;
}

/* This function name matches the vulnerable function in the real source. */
void png_read_filter_row_up_msa(png_row_info *row_info, png_byte *row, const png_byte *prev_row) {
    /* We directly simulate entry into the buggy tail path where (cnt16 && cnt) is true
       while only 17..31 bytes remain. This reproduces the problematic LD_UB2 loads
       that read 32 bytes from row and prev_row. */

    /* Set rp/pp to point to a tail of exactly 17 bytes remaining. */
    size_t istop = row_info->rowbytes;
    if (istop < 17) {
        return; /* nothing to do; caller should ensure >= 17 */
    }

    png_byte *rp = row + (istop - 17);
    const png_byte *pp = prev_row + (istop - 17);

    /* Force the (cnt16 && cnt) path */
    int cnt16 = 1;
    int cnt = 1;

    v16u8 src0, src1, src4, src5;

    if (cnt16 && cnt) {
        /* BUG: For only 17 bytes remaining, the following reads 32 bytes
           (16 from rp, 16 from rp+16), overrunning the tail by 15 bytes. */
        src0 = LD_UB(rp);          /* in-bounds 16-byte read */
        src1 = LD_UB(rp + 16);     /* out-of-bounds read (by up to 15 bytes) */

        src4 = LD_UB(pp);          /* in-bounds 16-byte read */
        src5 = LD_UB(pp + 16);     /* out-of-bounds read (by up to 15 bytes) */

        /* Do the vector add and store like the original code */
        src0 = ADD2_impl(src0, src4);
        src1 = ADD2_impl(src1, src5);

        ST_UB2(src0, src1, rp, 16); /* This may also write OOB, but reads happen first */
        rp += 32;
    }
}

int main(void) {
    /* Choose a row size where the final remaining tail is 17 bytes. */
    const size_t rowbytes = 49; /* arbitrary >= 17 */

    png_row_info info;
    info.rowbytes = rowbytes;

    /* Allocate exactly 'rowbytes' bytes for row and prev_row */
    png_byte *row = (png_byte*)malloc(rowbytes);
    png_byte *prev = (png_byte*)malloc(rowbytes);

    if (!row || !prev) {
        fprintf(stderr, "Allocation failure\n");
        return 1;
    }

    /* Initialize with non-zero data to avoid optimization and make behavior deterministic */
    for (size_t i = 0; i < rowbytes; i++) {
        row[i] = (unsigned char)(i & 0xFF);
        prev[i] = (unsigned char)((i * 3) & 0xFF);
    }

    /* Call into the function that contains the buggy tail handling. */
    png_read_filter_row_up_msa(&info, row, prev);

    /* Prevent compiler from optimizing everything away */
    volatile unsigned sum = 0;
    for (size_t i = 0; i < rowbytes; i++) sum += row[i];
    fprintf(stderr, "Checksum: %u\n", sum);

    free(row);
    free(prev);
    return 0;
}
