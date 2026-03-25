#include <stdint.h>
#include <klee/klee.h>

// Incomplete type is enough; we don't dereference AVSContext in the slice
typedef struct AVSContext AVSContext;

// Vulnerable function (neutralized to only keep the target case and vulnerable line)
void ff_cavs_load_intra_pred_luma(AVSContext *h, uint8_t *top,
                                  uint8_t **left, int block) {
    (void)h; (void)left; // unused in the sliced path
    // Original structure had a switch(block) with multiple cases; keep only target case (0)
    switch (block) {
        case 0:
            // Vulnerable statement (verbatim): potential OOB read if top has < 2 bytes
            top[0]  = top[1];
            // Universal sink assertion after the vulnerable access
            klee_assert(0 && "SAILOR_SINK_REACHED");
            break;
        default:
            break;
    }
}

// Entry function: MUST be a direct pass-through to the vulnerable function
int decode_entry(AVSContext *h, uint8_t *top, uint8_t **left, int block) {
    ff_cavs_load_intra_pred_luma(h, top, left, block);
    return 0;
}
