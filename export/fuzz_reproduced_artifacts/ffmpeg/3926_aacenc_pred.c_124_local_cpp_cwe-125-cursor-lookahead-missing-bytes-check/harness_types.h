/* AUTO-GENERATED from harness preamble */
#pragma once

/* harness/aacenc_pred.c */
#include <stddef.h>
#include <stdint.h>

#ifndef EIGHT_SHORT_SEQUENCE
#define EIGHT_SHORT_SEQUENCE 2
#endif

/* Minimal type slices to support the vulnerable access */
typedef struct IndividualChannelStream {
    int *window_sequence;  /* pointer so [0] can be OOB when under-allocated */
} IndividualChannelStream;

typedef struct SingleChannelElement {
    IndividualChannelStream ics;
} SingleChannelElement;

typedef struct AACEncContext {
    int samplerate_index;
} AACEncContext;

/* Vulnerable function (neutralized to only keep the vulnerable access) */
