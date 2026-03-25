/* harness.c — minimal sliced spine for ff_aac_adjust_common_pred */
#include <stdint.h>
#include <stdlib.h>
#include <klee/klee.h>

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
void ff_aac_adjust_common_pred(AACEncContext *s, ChannelElement *cpe)
{
    SingleChannelElement *sce0 = &cpe->ch[0];
    SingleChannelElement *sce1 = &cpe->ch[1];

    // Vulnerable condition — verbatim access to window_sequence[0]
    if (!cpe->common_window ||
        sce0->ics.window_sequence[0] == EIGHT_SHORT_SEQUENCE ||
        sce1->ics.window_sequence[0] == EIGHT_SHORT_SEQUENCE)
        ; // neutralized: do not return here to let sink fire below

    // Universal sink assertion to mark reachability if no crash occurred during the condition evaluation
    klee_assert(0 && "SAILOR_SINK_REACHED");
}

// Entry function — must be a direct pass-through with no guards
int entry_func(AACEncContext *s, ChannelElement *cpe) {
    ff_aac_adjust_common_pred(s, cpe);
    return 0;
}
