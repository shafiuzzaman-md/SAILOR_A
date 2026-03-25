#include <stdint.h>
#include <stdlib.h>
#include <klee/klee.h>

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
void ff_aac_search_for_tns(AACEncContext *s, SingleChannelElement *sce)
{
    const int is8 = sce->ics.window_sequence[0] == EIGHT_SHORT_SEQUENCE;  // aacenc_tns.c:167
    // Universal sink assertion after the vulnerable access
    klee_assert(0 && "SAILOR_SINK_REACHED");
}

// Entry function MUST be a pure pass-through (no guards)
int entry_func(AACEncContext *s, SingleChannelElement *sce) {
    ff_aac_search_for_tns(s, sce);
    return 0;
}
