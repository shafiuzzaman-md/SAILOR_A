/* AUTO-GENERATED from harness preamble */
#pragma once

/* harness.c — minimal sliced spine for ff_aac_adjust_common_pred */
#include <stdint.h>
#include <stdlib.h>

#ifndef EIGHT_SHORT_SEQUENCE
#define EIGHT_SHORT_SEQUENCE 2
#endif

// Minimal type defs required by the vulnerable function
typedef struct {
    int samplerate_index;
} AACEncContext;

typedef struct {
    // minimal subset used by the vulnerable check
    struct {
        int max_sfb;                 // unused in our slice but kept for shape
        uint8_t *window_sequence;    // accessed at index [0]
    } ics;
    // other fields omitted (not on the sliced path)
} SingleChannelElement;

typedef struct {
    int common_window;              // guard in the vulnerable check
    SingleChannelElement ch[2];     // ch[0], ch[1]
} ChannelElement;

// Vulnerable function (neutralized). Keep the exact vulnerable statement text.
