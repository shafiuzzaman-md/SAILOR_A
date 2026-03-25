#include <stdint.h>
#include <stdlib.h>
#include <klee/klee.h>

// Minimal local typedefs/macros to compile the slice
#ifndef MV_FWD_B2
#define MV_FWD_B2 1
#endif
#ifndef MV_BWD_B2
#define MV_BWD_B2 5
#endif

// Minimal cavs_vector type
typedef struct cavs_vector {
    int16_t x;
    int16_t y;
    int16_t ref;
} cavs_vector;

// Minimal AVSContext carrying only fields we use here
typedef struct AVSContext {
    int mbx;
    int mb_width;
    cavs_vector mv[64];
    cavs_vector *top_mv[2];
} AVSContext;

// Vulnerable function (neutralized) — keep only the for-loop with the exact vulnerable statement
void ff_cavs_init_mb(AVSContext *h)
{
    int i;
    /* copy predictors from top line (MB B and C) into cache */
    for (i = 0; i < 3; i++) {
        h->mv[MV_FWD_B2 + i] = h->top_mv[0][h->mbx * 2 + i];
        klee_assert(0 && "SAILOR_SINK_REACHED");
        h->mv[MV_BWD_B2 + i] = h->top_mv[1][h->mbx * 2 + i];
    }
}

// Mandatory pass-through entry — NO guards
int entry_func(AVSContext *h) {
    ff_cavs_init_mb(h);
    return 0;
}
