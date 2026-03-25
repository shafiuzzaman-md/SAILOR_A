/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdint.h>
#include <stdlib.h>

// Minimal local defines/macros
#ifndef EIGHT_SHORT_SEQUENCE
#define EIGHT_SHORT_SEQUENCE 2
#endif

// Minimal type slices to satisfy the vulnerable function
typedef struct LongTermPrediction {
    int present;
    int lag;
    int coef_idx;
    int used[128];
} LongTermPrediction;

typedef struct IndividualChannelStream {
    uint8_t *window_sequence; // pointer to allow potential OOB on [0]
    int max_sfb;
    LongTermPrediction ltp;
    int predictor_present;
} IndividualChannelStream;

typedef struct SingleChannelElement {
    IndividualChannelStream ics;
    float ltp_state[3072];
} SingleChannelElement;

typedef struct ChannelElement {
    int common_window;
    SingleChannelElement ch[2];
} ChannelElement;

typedef struct AACEncContext {
    int dummy; // not used in this slice
} AACEncContext;

// Vulnerable function (neutralized, keep the exact vulnerable statement)
