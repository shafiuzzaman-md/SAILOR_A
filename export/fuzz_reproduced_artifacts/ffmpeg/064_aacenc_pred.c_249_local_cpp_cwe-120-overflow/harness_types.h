/* AUTO-GENERATED from harness preamble */
#pragma once

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
