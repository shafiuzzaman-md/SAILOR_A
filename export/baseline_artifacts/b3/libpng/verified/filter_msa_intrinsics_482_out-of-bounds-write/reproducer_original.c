#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* Minimal type re-declarations matching libpng usage */
typedef unsigned char png_byte;
typedef struct png_row_info_struct {
    size_t width;
    size_t rowbytes;
} png_row_info;

/* Stub MSA vector types as simple byte arrays */
typedef struct { unsigned char v[16]; } v16u8;
typedef v16u8 v16i8;
typedef v16u8 v4i32;
typedef v16u8 v2i64;

/* Helper: load 32-bit word (unaligned) */
static inline int32_t LW(const png_byte *p) {
    uint32_t v = 0;
    memcpy(&v, p, 4);
    return (int32_t)v;
}

/* Stub intrinsics/macros used by the MSA path. These are no-ops or simple pass-throughs
   sufficient to preserve the control flow and the 16-byte store. */
static inline v16u8 __msa_insert_w(v4i32 a, int idx, int32_t w) {
    (void)idx; (void)w; /* content does not matter for triggering the bug */
    return a;
}
static inline v16u8 __msa_sldi_b(v16i8 a, v16i8 b, int sh) {
    (void)a; (void)sh;
    return b;
}
static inline v16u8 __msa_pckev_d(v2i64 a, v2i64 b) {
    (void)a;
    return b;
}

/* Vector add (byte-wise) to satisfy += style ops in scalar C */
static inline v16u8 add_v16u8(v16u8 x, v16u8 y) {
    v16u8 r;
    for (int i = 0; i < 16; i++) r.v[i] = (unsigned char)(x.v[i] + y.v[i]);
    return r;
}

/* Load 16 bytes "vector" from memory. To isolate the write OOB, we DO NOT actually
   dereference the pointer here; just return zeros. */
static inline v16u8 LD_UB(const png_byte *p) {
    (void)p;
    v16u8 r; memset(r.v, 0, 16); return r;
}

/* Interleave even words: stubbed to pass through inputs */
#define ILVEV_W2_UB(a, b, c, d, out0, out1) do { (out0) = (a); (out1) = (c); } while (0)

/* Store 16 bytes to memory: this is the operation that will write past the end */
static inline void ST_UB(v16u8 x, png_byte *p) {
    /* This memcpy of 16 bytes is the out-of-bounds write when p is near the end */
    memcpy(p, x.v, 16);
}

/* Re-implementation of the vulnerable function with the same control flow and tail bug */
static void png_read_filter_row_sub4_msa(png_row_info *row_info, png_byte *row,
    const png_byte *prev_row)
{
    (void)prev_row; /* unused in this path */
    size_t count;
    size_t istop = row_info->rowbytes;
    png_byte *src = row;
    png_byte *nxt = row + 4;
    int32_t inp0;
    v16u8 src0, src1, src2, src3, src4;
    v16u8 dst0, dst1;
    v16u8 zero; memset(&zero, 0, sizeof(zero));

    istop -= 4;

    inp0 = LW(src);
    src += 4;
    src0 = __msa_insert_w((v4i32) zero, 0, inp0);

    for (count = 0; count < istop; count += 16)
    {
        src1 = LD_UB(src);
        src += 16;

        src2 = __msa_sldi_b((v16i8) zero, (v16i8) src1, 4);
        src3 = __msa_sldi_b((v16i8) zero, (v16i8) src1, 8);
        src4 = __msa_sldi_b((v16i8) zero, (v16i8) src1, 12);
        src1 = add_v16u8(src1, src0);
        src2 = add_v16u8(src2, src1);
        src3 = add_v16u8(src3, src2);
        src4 = add_v16u8(src4, src3);
        src0 = src4;
        ILVEV_W2_UB(src1, src2, src3, src4, dst0, dst1);
        dst0 = __msa_pckev_d((v2i64) dst1, (v2i64) dst0);

        /* Vulnerable: always stores 16 bytes without checking tail */
        ST_UB(dst0, nxt);
        nxt += 16;
    }
}

int main(void) {
    /* Choose rowbytes so that (rowbytes - 4) % 16 == 1, maximizing overflow.
       rowbytes = 5 => istop = 1, loop runs once, writes 16 bytes at row+4,
       overflowing 15 bytes past the end of the 5-byte buffer. */
    size_t rowbytes = 5; /* Minimal size that still enters the loop once */
    png_row_info ri;
    ri.width = 0; /* not used here */
    ri.rowbytes = rowbytes;

    png_byte *row = (png_byte*)malloc(rowbytes);
    if (!row) {
        fprintf(stderr, "alloc failed\n");
        return 1;
    }
    memset(row, 0x41, rowbytes);

    /* prev_row is unused in this filter implementation */
    png_read_filter_row_sub4_msa(&ri, row, NULL);

    /* If we got here without ASan abort, free memory (unlikely). */
    free(row);
    return 0;
}
