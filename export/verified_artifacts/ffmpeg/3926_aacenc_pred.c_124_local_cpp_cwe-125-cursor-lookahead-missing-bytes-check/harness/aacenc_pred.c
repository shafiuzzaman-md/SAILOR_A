/* harness/aacenc_pred.c */
#include <klee/klee.h>
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
void ff_aac_apply_main_pred(AACEncContext *s, SingleChannelElement *sce)
{
    /* Vulnerable statement must be verbatim: */
    if (sce->ics.window_sequence[0] != EIGHT_SHORT_SEQUENCE) {
        /* After the vulnerable read, assert sink reachability */
        klee_assert(0 && "SAILOR_SINK_REACHED");
    } else {
        /* alternate path */
    }
}

/* Entry function: mandatory direct pass-through with no guards */
int harness_entry(AACEncContext *s, SingleChannelElement *sce) {
    ff_aac_apply_main_pred(s, sce);
    return 0;
}
