#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// Replicate the vulnerable pattern from libtiff/tif_pixarlog.c
// Key pieces: REPEAT that executes the body once even when n==0,
// and the horizontalAccumulate12 else-branch that relies on REPEAT(stride, ...)

#define CODE_MASK 0x0FFFu
#define SCALE12 1.0f

static inline uint16_t clamp12(float v) {
    if (v < 0.0f)
        v = 0.0f;
    if (v > 4095.0f)
        v = 4095.0f;
    return (uint16_t)(v + 0.5f);
}
#define CLAMP12(x) clamp12((x))

// Deliberately buggy REPEAT macro: executes the body once even when n == 0
// This mirrors the behavior described in the vulnerability explanation
#define REPEAT(n, op)                                                     \
    do {                                                                 \
        switch ((n)) {                                                   \
        default:                                                         \
            break;                                                       \
        case 4: op; /* fallthrough */                                    \
        case 3: op; /* fallthrough */                                    \
        case 2: op; /* fallthrough */                                    \
        case 1: op; /* fallthrough */                                    \
        case 0: op;                                                      \
            break;                                                       \
        }                                                                \
    } while (0)

static void horizontalAccumulate12(uint16_t *wp, int n, int stride,
                                   uint16_t *op, float *ToLinearF)
{
    unsigned int cr = 0, cg = 0, cb = 0, ca = 0, mask;
    float t0 = 0, t1 = 0, t2 = 0, t3 = 0;

    if (n >= stride) {
        mask = CODE_MASK;
        if (stride == 3) {
            // Not used in this reproducer
        } else if (stride == 4) {
            // Not used in this reproducer
        } else {
            // Vulnerable path when stride == 0
            REPEAT(stride, t0 = ToLinearF[*wp & mask] * SCALE12;
                           *op = CLAMP12(t0); wp++; op++);
            n -= stride; // With stride == 0, n is not decreased
            while (n > 0) {
                REPEAT(stride, wp[stride] += *wp;                         \
                               t0 = ToLinearF[wp[stride] & mask] * SCALE12; \
                               *op = CLAMP12(t0); wp++; op++);
                n -= stride; // With stride == 0, this never decreases n -> infinite loop
            }
        }
    }
}

int main(void) {
    // Build the ToLinearF table (size 4096 for 12-bit mask)
    float *ToLinearF = (float *)malloc(4096 * sizeof(float));
    if (!ToLinearF) {
        perror("malloc ToLinearF");
        return 1;
    }
    for (int i = 0; i < 4096; i++) {
        ToLinearF[i] = (float)i; // simple linear mapping
    }

    // Minimal input/output buffers of size 1 to trigger OOB quickly
    uint16_t *wp = (uint16_t *)malloc(1 * sizeof(uint16_t));
    uint16_t *op = (uint16_t *)malloc(1 * sizeof(uint16_t));
    if (!wp || !op) {
        perror("malloc buffers");
        return 1;
    }

    wp[0] = 1;
    op[0] = 0;

    // Trigger: n > 0, stride == 0
    // The first REPEAT executes once even though stride==0 and advances wp/op.
    // Then the while loop condition (n > 0) is true and, since n is never
    // decreased, the loop becomes infinite and immediately performs OOB
    // read/write on the incremented pointers.
    horizontalAccumulate12(wp, /*n=*/1, /*stride=*/0, op, ToLinearF);

    // We should never reach here due to ASan crash
    printf("Done (unexpected)\n");
    free(wp);
    free(op);
    free(ToLinearF);
    return 0;
}
