#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Minimal libpng-like typedefs used by the vulnerable function */
typedef unsigned char png_byte;
typedef struct {
    size_t rowbytes;
} png_row_info;

/* Minimal vector type to emulate 16-byte MSA vector loads/stores */
typedef struct { uint8_t b[16]; } v16u8;

static inline v16u8 load16(const uint8_t *p) {
    v16u8 v;
    /* This memcpy will trigger ASan on out-of-bounds reads */
    memcpy(v.b, p, 16);
    return v;
}

static inline void store16(uint8_t *p, v16u8 v) {
    memcpy(p, v.b, 16);
}

static inline v16u8 add_vec(v16u8 x, v16u8 y) {
    v16u8 r;
    for (int i = 0; i < 16; i++) r.b[i] = (uint8_t)(x.b[i] + y.b[i]);
    return r;
}

/* Emulate the used MSA helper macros with plain C */
#define LD_UB(p)           load16((const uint8_t*)(p))
#define ST_UB(v, p)        store16((uint8_t*)(p), (v))

#define LD_UB2(p, stride, a, b)                         \
    do {                                                \
        (a) = load16((const uint8_t*)(p) + 0*(stride)); \
        (b) = load16((const uint8_t*)(p) + 1*(stride)); \
    } while (0)

#define ST_UB2(a, b, p, stride)                         \
    do {                                                \
        store16((uint8_t*)(p) + 0*(stride), (a));       \
        store16((uint8_t*)(p) + 1*(stride), (b));       \
    } while (0)

#define LD_UB4(p, stride, a, b, c, d)                   \
    do {                                                \
        (a) = load16((const uint8_t*)(p) + 0*(stride)); \
        (b) = load16((const uint8_t*)(p) + 1*(stride)); \
        (c) = load16((const uint8_t*)(p) + 2*(stride)); \
        (d) = load16((const uint8_t*)(p) + 3*(stride)); \
    } while (0)

#define ST_UB4(a, b, c, d, p, stride)                   \
    do {                                                \
        store16((uint8_t*)(p) + 0*(stride), (a));       \
        store16((uint8_t*)(p) + 1*(stride), (b));       \
        store16((uint8_t*)(p) + 2*(stride), (c));       \
        store16((uint8_t*)(p) + 3*(stride), (d));       \
    } while (0)

#define ADD2(a0, b0, a1, b1, o0, o1)                    \
    do {                                                \
        (o0) = add_vec((a0), (b0));                     \
        (o1) = add_vec((a1), (b1));                     \
    } while (0)

#define ADD3(a0, b0, a1, b1, a2, b2, o0, o1, o2)        \
    do {                                                \
        (o0) = add_vec((a0), (b0));                     \
        (o1) = add_vec((a1), (b1));                     \
        (o2) = add_vec((a2), (b2));                     \
    } while (0)

#define ADD4(a0, b0, a1, b1, a2, b2, a3, b3, o0, o1, o2, o3) \
    do {                                                     \
        (o0) = add_vec((a0), (b0));                          \
        (o1) = add_vec((a1), (b1));                          \
        (o2) = add_vec((a2), (b2));                          \
        (o3) = add_vec((a3), (b3));                          \
    } while (0)

/* Buggy function extracted/simplified from mips/filter_msa_intrinsics.c */
static void png_read_filter_row_up_msa(png_row_info *row_info, png_byte *row,
                                       const png_byte *prev_row)
{
    size_t cnt, cnt16, cnt32;
    size_t istop = row_info->rowbytes;
    png_byte *rp = row;
    const png_byte *pp = prev_row;
    v16u8 src0, src1, src2, src3, src4, src5, src6, src7;

    /* We intentionally omit the 64-byte main loop to focus on the tail. */

    if (istop & 0x3F)
    {
        cnt32 = istop & 0x20;
        cnt16 = istop & 0x10;
        cnt = istop & 0xF;

        if (cnt32)
        {
            if (cnt16 && cnt)
            {
                /* Vulnerable tail: reads 4x16=64 bytes when only 49..63 remain */
                LD_UB4(rp, 16, src0, src1, src2, src3);   /* overreads row */
                LD_UB4(pp, 16, src4, src5, src6, src7);   /* overreads prev_row */

                ADD4(src0, src4, src1, src5, src2, src6, src3, src7,
                     src0, src1, src2, src3);

                ST_UB4(src0, src1, src2, src3, rp, 16);
            }
            else if (cnt16 || cnt)
            {
                /* Not taken in our repro, but implemented for completeness */
                LD_UB2(rp, 16, src0, src1);
                LD_UB2(pp, 16, src4, src5);
                src2 = LD_UB(rp + 32);
                src6 = LD_UB(pp + 32);
                ADD3(src0, src4, src1, src5, src2, src6, src0, src1, src2);
                ST_UB2(src0, src1, rp, 16);
                ST_UB(src2, rp + 32);
            }
            else
            {
                /* Not taken in our repro, but implemented for completeness */
                LD_UB2(rp, 16, src0, src1);
                LD_UB2(pp, 16, src4, src5);
                ADD2(src0, src4, src1, src5, src0, src1);
                ST_UB2(src0, src1, rp, 16);
            }
        }
        else if (cnt16 && cnt)
        {
            /* Other tails - not used in this repro */
            LD_UB2(rp, 16, src0, src1);
            LD_UB2(pp, 16, src4, src5);
            ADD2(src0, src4, src1, src5, src0, src1);
            ST_UB2(src0, src1, rp, 16);
            ST_UB(LD_UB(rp + 32), rp + 32); /* dummy op to keep structure similar */
        }
        else if (cnt16 || cnt)
        {
            /* Minimal no-op to keep compiler happy; not part of the bug path. */
        }
    }
}

int main(void)
{
    /* Choose rowbytes so that: cnt32!=0, cnt16!=0, cnt!=0 -> triggers buggy path.
       Example: 49 = 0x31 = 0x20 + 0x10 + 0x01. */
    const size_t rowbytes = 49;  /* 49..63 all trigger the same buggy tail */

    png_byte *row = (png_byte*)malloc(rowbytes);
    png_byte *prev = (png_byte*)malloc(rowbytes);
    if (!row || !prev) {
        fprintf(stderr, "malloc failed\n");
        return 1;
    }

    /* Initialize buffers with some data */
    for (size_t i = 0; i < rowbytes; i++) {
        row[i] = (png_byte)(i & 0xFF);
        prev[i] = (png_byte)((255 - i) & 0xFF);
    }

    png_row_info info;
    info.rowbytes = rowbytes;

    /* This call will perform 4x16B loads (64 bytes) from row and prev
       even though only 49 bytes remain, causing out-of-bounds reads
       detectable by AddressSanitizer. */
    png_read_filter_row_up_msa(&info, row, prev);

    /* Prevent optimization from discarding results */
    volatile uint8_t sink = row[0];
    (void)sink;

    free(row);
    free(prev);
    return 0;
}