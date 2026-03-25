#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Mimic the macro from libtiff/tif_pixarlog.c that executes its body once
 * even when asked to repeat 0 times. This is the crux of the bug. */
#define REPEAT(n, body)            \
    do {                          \
        int __rep = (int)(n);     \
        do {                      \
            body;                 \
        } while (--__rep > 0);    \
    } while (0)

/* Same mask constant used by the PixarLog code. */
#define CODE_MASK 0x0FFFu

/* Vulnerable function extracted/adapted from libtiff/tif_pixarlog.c
 * horizontalAccumulateF(...) showing the buggy REPEAT(stride, ...) usage. */
static void horizontalAccumulateF(uint16_t *wp, int n, int stride, float *op,
                                  float *ToLinearF)
{
    unsigned int cr = 0, cg = 0, cb = 0, ca = 0, mask;
    float t0, t1, t2, t3;

    if (n >= stride)
    {
        mask = CODE_MASK;
        if (stride == 3)
        {
            /* 3-channel branch (not used in this reproducer) */
            t0 = ToLinearF[cr = (wp[0] & mask)];
            t1 = ToLinearF[cg = (wp[1] & mask)];
            t2 = ToLinearF[cb = (wp[2] & mask)];
            op[0] = t0;
            op[1] = t1;
            op[2] = t2;
            n -= 3;
            while (n > 0)
            {
                wp += 3;
                op += 3;
                n -= 3;
                t0 = ToLinearF[(cr += wp[0]) & mask];
                t1 = ToLinearF[(cg += wp[1]) & mask];
                t2 = ToLinearF[(cb += wp[2]) & mask];
                op[0] = t0;
                op[1] = t1;
                op[2] = t2;
            }
        }
        else if (stride == 4)
        {
            /* 4-channel branch (not used in this reproducer) */
            t0 = ToLinearF[cr = (wp[0] & mask)];
            t1 = ToLinearF[cg = (wp[1] & mask)];
            t2 = ToLinearF[cb = (wp[2] & mask)];
            t3 = ToLinearF[ca = (wp[3] & mask)];
            op[0] = t0;
            op[1] = t1;
            op[2] = t2;
            op[3] = t3;
            n -= 4;
            while (n > 0)
            {
                wp += 4;
                op += 4;
                n -= 4;
                t0 = ToLinearF[(cr += wp[0]) & mask];
                t1 = ToLinearF[(cg += wp[1]) & mask];
                t2 = ToLinearF[(cb += wp[2]) & mask];
                t3 = ToLinearF[(ca += wp[3]) & mask];
                op[0] = t0;
                op[1] = t1;
                op[2] = t2;
                op[3] = t3;
            }
        }
        else
        {
            /* Vulnerable path: when stride == 0, REPEAT executes once,
             * then n -= stride leaves n unchanged, and the while(n>0)
             * loop never terminates, walking op/wp out of bounds. */
            REPEAT(stride, *op = ToLinearF[*wp & mask]; wp++; op++);
            n -= stride; /* if stride == 0, n stays the same */
            while (n > 0)
            {
                REPEAT(stride, wp[stride] += *wp; *op = ToLinearF[*wp & mask];
                       wp++; op++);
                n -= stride; /* still unchanged when stride == 0 */
            }
        }
    }
}

int main(void)
{
    /* Craft inputs that model the bad state produced by an invalid
     * SamplesPerPixel (stride == 0) and a positive n (e.g., image width).
     * Small buffers ensure ASan will detect out-of-bounds quickly. */
    const int stride = 0;   /* corresponds to invalid SamplesPerPixel == 0 */
    const int n = 8;        /* positive, so the while(n>0) never terminates */

    /* Allocate tiny buffers. The vulnerable loop will walk off these. */
    uint16_t *wp = (uint16_t *)malloc(sizeof(uint16_t));
    float *op = (float *)malloc(sizeof(float));
    if (!wp || !op)
    {
        fprintf(stderr, "alloc failed\n");
        return 1;
    }
    *wp = 1; /* any small code */

    /* Build a ToLinearF lookup table with CODE_MASK+1 entries. */
    float *ToLinearF = (float *)malloc((CODE_MASK + 1) * sizeof(float));
    if (!ToLinearF)
    {
        fprintf(stderr, "alloc failed\n");
        return 1;
    }
    for (unsigned i = 0; i <= CODE_MASK; ++i)
        ToLinearF[i] = (float)i;

    /* This call will run the buggy path and walk off the end of op/wp. */
    horizontalAccumulateF(wp, n, stride, op, ToLinearF);

    /* Not reached if ASan catches the out-of-bounds first. */
    free(ToLinearF);
    free(op);
    free(wp);
    return 0;
}
