#include <stdint.h>
#include <stddef.h>
// Combined reproducer for 4982_cavs.c_365_local_cpp_cwe-125-cursor-lookahead-missing-bytes-check
// Original harness: driver.c + smart_stubs.c + sliced source

// === smart_stubs.c ===
/* Smart stubs — auto-generated from path + vulnerability analysis */
/* Symbolic stubs model the environment: KLEE explores return values */
/* that both REACH the sink AND TRIGGER the vulnerability */
#include <stdlib.h>
#include <string.h>
// === driver.c ===
#include "harness_types.h"
#include <stdlib.h>
#include <string.h>

// Entry prototype from harness
int cavs_entry(AVSContext *h, int *pred_mode_uv);

int LLVMFuzzerTestOneInput(const uint8_t *fuzz_data, size_t fuzz_size) {
    if (fuzz_size < 64) return 0;
    // Allocate context
    AVSContext *h = (AVSContext *)calloc(1, sizeof(AVSContext));

    // Allocate pred_mode_Y with ONLY 5 ints (valid indices 0..4). Index 5 will be OOB
    size_t y_count = 5;  // concrete size per instructions
    int *pred_y = (int *)malloc(y_count * sizeof(int));
    // Make contents symbolic (not strictly needed for OOB, but consistent)
    memcpy(pred_y, fuzz_data + (0), y_count * sizeof(int));

    // Hook into context
    h->pred_mode_Y = pred_y;

    // Initialize other fields present in the struct
    h->top_pred_Y = (int *)calloc(2, sizeof(int));
    h->flags = 0;
    h->mbx = 0;

    // pred_mode_uv argument: allocate one int
    int *pred_mode_uv = (int *)malloc(sizeof(int));
    memcpy(pred_mode_uv, fuzz_data + (y_count * sizeof(int)), sizeof(int));

    // Call entry (pass-through to vulnerable function)
    cavs_entry(h, pred_mode_uv);
    return 0;
}
