#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* This reproducer embeds the vulnerable logic from libtiff/tif_pixarlog.c
 * horizontalAccumulate11 else-branch together with a REPEAT macro
 * that executes the body once when n == 0, matching the vulnerability
 * description. When called with stride == 0 and n > 0, the outer while(n>0)
 * loop never terminates and keeps incrementing wp/op, causing OOB writes. */

/* In tif_pixarlog.c this is CODE_MASK for the 11-bit path */
#define CODE_MASK 0x7FFu

/* REPEAT macro that executes the body once even when count (n) is 0.
 * This mirrors the described buggy behavior where REPEAT(stride, ...) executes once if stride == 0. */
#define REPEAT(n, op)                        \
    do {                                     \
        switch ((n)) {                        \
        case 0:                               \
            op;                               \
            break;                            \
        case 1:                               \
            op;                               \
            break;                            \
        case 2:                               \
            op; op;                           \
            break;                            \
        case 3:                               \
            op; op; op;                       \
            break;                            \
        case 4:                               \
            op; op; op; op;                   \
            break;                            \
        default: {                            \
            int _i_;                          \
            for (_i_ = 0; _i_ < (n); _i_++) { \
                op;                           \
            }                                  \
        }                                      \
        }                                      \
    } while (0)

static void horizontalAccumulate11(uint16_t *wp, int n, int stride, uint16_t *op)
{
    unsigned int cr, cg, cb, ca, mask;

    if (n >= stride)
    {
        mask = CODE_MASK;
        if (stride == 3)
        {
            /* Not used in this reproducer, but kept to mirror structure */
            op[0] = wp[0];
            op[1] = wp[1];
            op[2] = wp[2];
            cr = wp[0];
            cg = wp[1];
            cb = wp[2];
            n -= 3;
            while (n > 0)
            {
                wp += 3;
                op += 3;
                n -= 3;
                op[0] = (uint16_t)((cr += wp[0]) & mask);
                op[1] = (uint16_t)((cg += wp[1]) & mask);
                op[2] = (uint16_t)((cb += wp[2]) & mask);
            }
        }
        else if (stride == 4)
        {
            /* Not used in this reproducer, but kept to mirror structure */
            op[0] = wp[0];
            op[1] = wp[1];
            op[2] = wp[2];
            op[3] = wp[3];
            cr = wp[0];
            cg = wp[1];
            cb = wp[2];
            ca = wp[3];
            n -= 4;
            while (n > 0)
            {
                wp += 4;
                op += 4;
                n -= 4;
                op[0] = (uint16_t)((cr += wp[0]) & mask);
                op[1] = (uint16_t)((cg += wp[1]) & mask);
                op[2] = (uint16_t)((cb += wp[2]) & mask);
                op[3] = (uint16_t)((ca += wp[3]) & mask);
            }
        }
        else
        {
            /* Vulnerable path: with stride == 0, REPEAT executes once, n is not reduced,
             * and the while (n > 0) loop never terminates, incrementing wp/op forever. */
            REPEAT(stride, *op = *wp & mask; wp++; op++);
            n -= stride; /* n unchanged if stride == 0 */
            while (n > 0)
            {
                REPEAT(stride, wp[stride] += *wp; *op = *wp & mask; wp++; op++);
                n -= stride; /* still unchanged if stride == 0 -> infinite loop */
            }
        }
    }
}

int main(void)
{
    /* Allocate minimal buffers; ASan will detect OOB as the loop runs past them. */
    int n = 1;           /* Must be > 0 so while (n > 0) is entered */
    int stride = 0;      /* Triggers the buggy REPEAT/while interaction */

    uint16_t *wp = (uint16_t *)malloc(sizeof(uint16_t) * 1);
    uint16_t *op = (uint16_t *)malloc(sizeof(uint16_t) * 1);
    if (!wp || !op) {
        fprintf(stderr, "alloc failed\n");
        return 1;
    }
    wp[0] = 123; /* any non-zero value */
    op[0] = 0;

    /* This call will never return under normal execution, but with ASan enabled
     * it will quickly report an out-of-bounds write as op/wp run off the buffers. */
    horizontalAccumulate11(wp, n, stride, op);

    /* Should not reach here. */
    printf("Done: op[0]=%u\n", (unsigned)op[0]);

    free(wp);
    free(op);
    return 0;
}
