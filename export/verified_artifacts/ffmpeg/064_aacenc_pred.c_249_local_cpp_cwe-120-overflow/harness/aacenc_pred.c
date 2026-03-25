#include <klee/klee.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

// Opaque encoder context
typedef struct AACEncContext AACEncContext;

// Minimal predictor state (opaque for our slice)
typedef struct PredictorState {
    int dummy;
} PredictorState;

// Minimal SingleChannelElement with only needed fields
struct SingleChannelElement {
    float *prcoeffs;
    float *coeffs;
    PredictorState *predictor_state;
    struct {
        int predictor_initialized;
    } ics;
};

static inline void reset_all_predictors(PredictorState *ps) {
    (void)ps; // neutralized
}

// Vulnerable function slice: keep only the path to the memcpy sink
void ff_aac_search_for_pred(AACEncContext *s, struct SingleChannelElement *sce)
{
    (void)s; // unused in this slice
    if (!sce->ics.predictor_initialized) {
        reset_all_predictors(sce->predictor_state);
        sce->ics.predictor_initialized = 1;
        // Vulnerable statement copied verbatim from source context:
        memcpy(sce->prcoeffs, sce->coeffs, 1024*sizeof(float));
        // Universal sink assertion (fires if memcpy didn't crash):
        klee_assert(0 && "SAILOR_SINK_REACHED");
    }
}

// Entry function: direct pass-through
int harness_entry(AACEncContext *s, struct SingleChannelElement *sce) {
    ff_aac_search_for_pred(s, sce);
    return 0;
}
