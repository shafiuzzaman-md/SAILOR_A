/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdint.h>
#include <stdlib.h>

#ifndef EIGHT_SHORT_SEQUENCE
#define EIGHT_SHORT_SEQUENCE 2
#endif

// Minimal type defs needed for the vulnerable expression
typedef struct {
    int *window_sequence;  // minimal: only what's used by the vulnerable line
} IndividualChannelStream;

typedef struct {
    int present; // unused in our slice but part of sce->tns
} TemporalNoiseShaping;

typedef struct {
    IndividualChannelStream ics;
    TemporalNoiseShaping tns;
} SingleChannelElement;

typedef struct {
    int dummy; // placeholder, not used in this slice
} AACEncContext;

// Vulnerable function slice — keep only the vulnerable statement verbatim
