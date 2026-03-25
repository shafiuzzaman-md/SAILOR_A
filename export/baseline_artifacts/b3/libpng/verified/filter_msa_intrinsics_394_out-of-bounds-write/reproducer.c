#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Minimal libpng-like types */
typedef unsigned char png_byte;
typedef struct {
    size_t rowbytes;
} png_row_info;

/* Minimal MSA-like vector and intrinsics stubs */
typedef struct { unsigned char v[16]; } v16u8;

static inline v16u8 vec_zero(void) {
    v16u8 z; memset(z.v, 0, sizeof(z.v)); return z; }

/* Stub loads: do NOT actually read from memory to avoid OOB reads masking the write */
#define LD_UB4(p, stride, a, b, c, d) do { \
    (a) = vec_zero(); (b) = vec_zero(); (c) = vec_zero(); (d) = vec_zero(); \
} while (0)
#define LD_UB2(p, stride, a, b) do { \
    (a) = vec_zero(); (b) = vec_zero(); \
} while (0)
static inline v16u8 LD_UB(const png_byte *p) { (void)p; return vec_zero(); }

/* Adds: element-wise add with wrap-around */
static inline v16u8 add2(v16u8 x, v16u8 y) {
    v16u8 r; for (int i = 0; i < 16; i++) r.v[i] = (png_byte)(x.v[i] + y.v[i]); return r;
}
#define ADD4(a0,b0,a1,b1,a2,b2,a3,b3,o0,o1,o2,o3) do { \
    (o0) = add2((a0),(b0)); \
    (o1) = add2((a1),(b1)); \
    (o2) = add2((a2),(b2)); \
    (o3) = add2((a3),(b3)); \
} while (0)
#define ADD3(a0,b0,a1,b1,a2,b2,o0,o1,o2) do { \
    (o0) = add2((a0),(b0)); \
    (o1) = add2((a1),(b1)); \
    (o2) = add2((a2),(b2)); \
} while (0)
#define ADD2(a0,b0,a1,b1,o0,o1) do { \
    (o0) = add2((a0),(b0)); \
    (o1) = add2((a1),(b1)); \
} while (0)

/* Stores: actually write to memory to surface the OOB write */
#define ST_UB4(a,b,c,d,p,stride) do { \
    memcpy((p) + 0*(stride), (a).v, 16); \
    memcpy((p) + 1*(stride), (b).v, 16); \
    memcpy((p) + 2*(stride), (c).v, 16); \
    memcpy((p) + 3*(stride), (d).v, 16); \
} while (0)
#define ST_UB2(a,b,p,stride) do { \
    memcpy((p) + 0*(stride), (a).v, 16); \
    memcpy((p) + 1*(stride), (b).v, 16); \
} while (0)
#define ST_UB(a,p) do { \
    memcpy((p), (a).v, 16); \
} while (0)

/* Buggy function extracted and minimally adapted from mips/filter_msa_intrinsics.c */
static void png_read_filter_row_up_msa(png_row_info *row_info, png_byte *row, const png_byte *prev_row)
{
    size_t i, cnt, cnt16, cnt32;
    size_t istop = row_info->rowbytes;
    png_byte *rp = row;
    const png_byte *pp = prev_row;
    v16u8 src0, src1, src2, src3, src4, src5, src6, src7;

    for (i = 0; i < (istop >> 6); i++)
    {
        LD_UB4(rp, 16, src0, src1, src2, src3);
        LD_UB4(pp, 16, src4, src5, src6, src7);
        pp += 64;

        ADD4(src0, src4, src1, src5, src2, src6, src3, src7,
             src0, src1, src2, src3);

        ST_UB4(src0, src1, src2, src3, rp, 16);
        rp += 64;
    }

    if (istop & 0x3F)
    {
        cnt32 = istop & 0x20;
        cnt16 = istop & 0x10;
        cnt = istop & 0xF;

        if (cnt32)
        {
            if (cnt16 && cnt)
            {
                /* This path is buggy: it writes 64 bytes even when only 49-63 remain */
                LD_UB4(rp, 16, src0, src1, src2, src3);
                LD_UB4(pp, 16, src4, src5, src6, src7);

                ADD4(src0, src4, src1, src5, src2, src6, src3, src7,
                     src0, src1, src2, src3);

                ST_UB4(src0, src1, src2, src3, rp, 16); /* OOB write when remainder < 64 */
                rp += 64;
            }
            else if (cnt16 || cnt)
            {
                LD_UB2(rp, 16, src0, src1);
                LD_UB2(pp, 16, src4, src5);
                pp += 32;
                src2 = LD_UB(rp + 32);
                src6 = LD_UB(pp);

                ADD3(src0, src4, src1, src5, src2, src6, src0, src1, src2);

                ST_UB2(src0, src1, rp, 16);
                rp += 32;
                ST_UB(src2, rp);
                rp += 16;
            }
            else
            {
                LD_UB2(rp, 16, src0, src1);
                LD_UB2(pp, 16, src4, src5);

                ADD2(src0, src4, src1, src5, src0, src1);

                ST_UB2(src0, src1, rp, 16);
                rp += 32;
            }
        }
        else if (cnt16 && cnt)
        {
            LD_UB2(rp, 16, src0, src1);
            LD_UB2(pp, 16, src4, src5);

            ADD2(src0, src4, src1, src5, src0, src1);

            ST_UB2(src0, src1, rp, 16);
            rp += 32;
        }
        else if (cnt16 || cnt)
        {
            /* Not taken in our triggering case; left as no-op-ish to compile */
            src0 = LD_UB(rp);
            src1 = LD_UB(pp);
            ADD2(src0, src1, src0, src1, src0, src1);
            ST_UB(src0, rp);
        }
    }
}

int main(void)
{
    /* Choose a row size that satisfies: (istop & 0x20) && (istop & 0x10) && (istop & 0xF) */
    /* 49 = 32 + 16 + 1, so remainder is 49 bytes but code will write 64 bytes (overflow by 15). */
    const size_t rowbytes = 49;

    png_row_info info;
    info.rowbytes = rowbytes;

    /* Allocate only rowbytes for the destination to make the OOB write detectable. */
    png_byte *row = (png_byte *)malloc(rowbytes);
    if (!row) { perror("malloc row"); return 1; }
    memset(row, 0xA5, rowbytes);

    /* prev_row can be any buffer; allocate 64 to be safe though loads are stubbed. */
    png_byte *prev = (png_byte *)malloc(64);
    if (!prev) { perror("malloc prev"); return 1; }
    memset(prev, 0x5A, 64);

    /* Trigger the buggy tail path */
    png_read_filter_row_up_msa(&info, row, prev);

    /* If ASan didn't abort (it should), clean up. */
    free(prev);
    free(row);

    printf("Done (if you see this without ASan abort, the repro didn't trigger).\n");
    return 0;
}