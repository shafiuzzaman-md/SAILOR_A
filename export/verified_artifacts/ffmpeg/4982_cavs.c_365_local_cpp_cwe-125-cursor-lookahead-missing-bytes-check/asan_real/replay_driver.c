// Combined reproducer for 4982_cavs.c_365_local_cpp_cwe-125-cursor-lookahead-missing-bytes-check
// Original harness: driver.c + smart_stubs.c + sliced source

// === smart_stubs.c ===
/* Smart stubs — auto-generated from path + vulnerability analysis */
/* Symbolic stubs model the environment: KLEE explores return values */
/* that both REACH the sink AND TRIGGER the vulnerability */
#include <stdlib.h>
#include <string.h>
#include <klee/klee.h>

// === driver.c ===
#include "harness_types.h"
#include <klee/klee.h>
#include <stdlib.h>
#include <string.h>

// Entry prototype from harness
int cavs_entry(AVSContext *h, int *pred_mode_uv);

int main() {
    // Allocate context
    AVSContext *h = (AVSContext *)calloc(1, sizeof(AVSContext));

    // Allocate pred_mode_Y with ONLY 5 ints (valid indices 0..4). Index 5 will be OOB
    size_t y_count = 5;  // concrete size per instructions
    int *pred_y = (int *)malloc(y_count * sizeof(int));
    // Make contents symbolic (not strictly needed for OOB, but consistent)
    klee_make_symbolic(pred_y, y_count * sizeof(int), "pred_mode_Y_buf");

    // Hook into context
    h->pred_mode_Y = pred_y;

    // Initialize other fields present in the struct
    h->top_pred_Y = (int *)calloc(2, sizeof(int));
    h->flags = 0;
    h->mbx = 0;

    // pred_mode_uv argument: allocate one int
    int *pred_mode_uv = (int *)malloc(sizeof(int));
    klee_make_symbolic(pred_mode_uv, sizeof(int), "pred_mode_uv");

    // Call entry (pass-through to vulnerable function)
    cavs_entry(h, pred_mode_uv);
    return 0;
}
