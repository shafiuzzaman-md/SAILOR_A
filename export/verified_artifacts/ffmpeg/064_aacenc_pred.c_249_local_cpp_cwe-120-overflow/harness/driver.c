#include "harness_types.h"
#include <klee/klee.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

// harness_entry is defined in harness/aacenc_pred.c
int harness_entry(struct AACEncContext *s, struct SingleChannelElement *sce);

int main() {
    // Allocate SingleChannelElement and sub-objects concretely
    struct SingleChannelElement *sce = calloc(1, sizeof(*sce));
    if (!sce) return 0;

    // Ensure predictor is not initialized so memcpy path is taken
    sce->ics.predictor_initialized = 0;

    // predictor_state (unused by slice but must be non-null)
    sce->predictor_state = calloc(1, sizeof(*sce->predictor_state));

    // Allocate coeffs with full 1024 floats (source buffer)
    size_t src_count = 1024;
    sce->coeffs = malloc(src_count * sizeof(float));
    if (!sce->coeffs) return 0;
    klee_make_symbolic(sce->coeffs, src_count * sizeof(float), "coeffs_bytes");

    // Allocate prcoeffs too small to trigger overflow during memcpy
    size_t dst_count = 16; // intentionally smaller than 1024
    sce->prcoeffs = malloc(dst_count * sizeof(float));
    if (!sce->prcoeffs) return 0;
    klee_make_symbolic(sce->prcoeffs, dst_count * sizeof(float), "prcoeffs_bytes");

    // Call entry (s is unused in slice; pass NULL)
    harness_entry(NULL, sce);
    return 0;
}
