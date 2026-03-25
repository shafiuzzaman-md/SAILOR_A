#include "harness_types.h"
#include <klee/klee.h>
#include <stdlib.h>

int ff_cavs_init_pic(AVSContext *h);

int main() {
    // Intentionally under-allocate to force OOB at the vulnerable statement
    // AVSContext here contains only: int pred_mode_Y[9];
    // Allocate too few bytes so h->pred_mode_Y[3] write goes out-of-bounds
    size_t tiny = 8;  // concrete small size
    AVSContext *h = (AVSContext *)calloc(1, tiny);
    if (!h) return 0;

    // Optionally make the tiny region symbolic; not required for crash
    klee_make_symbolic(h, tiny, "avs_ctx_tiny");

    // Direct call to the vulnerable function
    ff_cavs_init_pic(h);
    return 0;
}
