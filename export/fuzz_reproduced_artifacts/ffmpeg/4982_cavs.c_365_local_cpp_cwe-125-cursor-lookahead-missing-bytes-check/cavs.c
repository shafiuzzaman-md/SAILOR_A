#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <klee/klee.h>

// Minimal local type mirroring only what's needed on the path
typedef struct AVSContext {
    int *pred_mode_Y;  // pointer so KLEE can track bounds
    int *top_pred_Y;   // unused in our sliced path
    int flags;         // unused in our sliced path
    int mbx;           // unused in our sliced path
} AVSContext;

// Vulnerable function — keep signature and the exact vulnerable statement
void ff_cavs_modify_mb_i(AVSContext *h, int *pred_mode_uv)
{
    /* save pred modes before they get modified */
    h->pred_mode_Y[3]             = h->pred_mode_Y[5];
    // Universal sink assertion — fires if statement didn't already crash
    klee_assert(0 && "SAILOR_SINK_REACHED");
}

// ENTRY FUNCTION — must be a pure pass-through to the vulnerable function
int cavs_entry(AVSContext *h, int *pred_mode_uv) {
    ff_cavs_modify_mb_i(h, pred_mode_uv);
    return 0;
}
