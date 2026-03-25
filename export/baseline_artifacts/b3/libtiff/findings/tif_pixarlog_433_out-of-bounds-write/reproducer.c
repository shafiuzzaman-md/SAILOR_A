#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Minimal definitions to mirror the vulnerable code path */
#define CODE_MASK 0x0FFFu

/* A REPEAT macro that executes the body once when n == 0, matching the buggy behavior */
#define REPEAT(n, OP) do { \
    switch (n) { \
        default: \
        case 8: { OP; } /* fallthrough */ \
        case 7: { OP; } /* fallthrough */ \
        case 6: { OP; } /* fallthrough */ \
        case 5: { OP; } /* fallthrough */ \
        case 4: { OP; } /* fallthrough */ \
        case 3: { OP; } /* fallthrough */ \
        case 2: { OP; } /* fallthrough */ \
        case 1: { OP; } /* fallthrough */ \
        case 0: { OP; } \
    } \
} while (0)

/* This is a standalone reproduction of the vulnerable function's relevant logic. */
static void horizontalAccumulate8(uint16_t *wp, int n, int stride,
                                  unsigned char *op,
                                  unsigned char *ToLinear8)
{
    unsigned int cr, cg, cb, ca, mask;

    (void)cr; (void)cg; (void)cb; (void)ca; /* silence unused warnings */

    if (n >= stride)
    {
        mask = CODE_MASK;
        if (stride == 3)
        {
            /* Not taken in this reproducer */
            op[0] = ToLinear8[cr = (wp[0] & mask)];
            op[1] = ToLinear8[cg = (wp[1] & mask)];
            op[2] = ToLinear8[cb = (wp[2] & mask)];
            n -= 3;
            while (n > 0)
            {
                n -= 3; wp += 3; op += 3;
                op[0] = ToLinear8[(cr += wp[0]) & mask];
                op[1] = ToLinear8[(cg += wp[1]) & mask];
                op[2] = ToLinear8[(cb += wp[2]) & mask];
            }
        }
        else if (stride == 4)
        {
            /* Not taken in this reproducer */
            op[0] = ToLinear8[cr = (wp[0] & mask)];
            op[1] = ToLinear8[cg = (wp[1] & mask)];
            op[2] = ToLinear8[cb = (wp[2] & mask)];
            op[3] = ToLinear8[ca = (wp[3] & mask)];
            n -= 4;
            while (n > 0)
            {
                n -= 4; wp += 4; op += 4;
                op[0] = ToLinear8[(cr += wp[0]) & mask];
                op[1] = ToLinear8[(cg += wp[1]) & mask];
                op[2] = ToLinear8[(cb += wp[2]) & mask];
                op[3] = ToLinear8[(ca += wp[3]) & mask];
            }
        }
        else
        {
            /* Vulnerable path when stride == 0 */
            REPEAT(stride, *op = ToLinear8[*wp & mask]; wp++; op++);
            n -= stride; /* n unchanged when stride == 0 */
            while (n > 0)
            {
                REPEAT(stride, wp[stride] += *wp; *op = ToLinear8[*wp & mask]; wp++; op++);
                n -= stride; /* still unchanged when stride == 0 => infinite loop */
            }
        }
    }
}

int main(void)
{
    /* Build a ToLinear8 table large enough for CODE_MASK */
    size_t lut_size = (size_t)CODE_MASK + 1; /* 4096 */
    unsigned char *ToLinear8 = (unsigned char *)malloc(lut_size);
    if (!ToLinear8) {
        fprintf(stderr, "alloc ToLinear8 failed\n");
        return 1;
    }
    for (size_t i = 0; i < lut_size; i++) {
        ToLinear8[i] = (unsigned char)(i & 0xFF);
    }

    /* Tiny output buffer so the infinite loop will immediately overflow */
    unsigned char *op = (unsigned char *)malloc(1);
    if (!op) {
        fprintf(stderr, "alloc op failed\n");
        free(ToLinear8);
        return 1;
    }
    op[0] = 0xAA;

    /* Minimal input buffer; will also be walked out-of-bounds by the loop */
    uint16_t *wp = (uint16_t *)malloc(sizeof(uint16_t));
    if (!wp) {
        fprintf(stderr, "alloc wp failed\n");
        free(op);
        free(ToLinear8);
        return 1;
    }
    wp[0] = 0; /* so ToLinear8 index stays in-bounds initially */

    /* Craft parameters to hit the buggy path: stride == 0 and n > 0 */
    int n = 10;      /* any positive value */
    int stride = 0;  /* triggers REPEAT to run once and n never decreases */

    fprintf(stderr, "Triggering horizontalAccumulate8 with stride==0 and n>0...\n");
    /* This call does not return before ASan reports an overflow */
    horizontalAccumulate8(wp, n, stride, op, ToLinear8);

    /* Not reached */
    free(wp);
    free(op);
    free(ToLinear8);
    return 0;
}
