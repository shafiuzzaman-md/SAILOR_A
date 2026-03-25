#include <stdint.h>
#include <stdlib.h>
#include <klee/klee.h>

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
void ff_aac_adjust_common_ltp(AACEncContext *s, ChannelElement *cpe)
{
    SingleChannelElement *sce0 = &cpe->ch[0];
    SingleChannelElement *sce1 = &cpe->ch[1];

    if (!cpe->common_window ||
        sce0->ics.window_sequence[0] == EIGHT_SHORT_SEQUENCE ||
        sce1->ics.window_sequence[0] == EIGHT_SHORT_SEQUENCE) {
        sce0->ics.ltp.present = 0;
        return;
    }
    // Universal sink assertion placed after the vulnerable read
    klee_assert(0 && "SAILOR_SINK_REACHED");
}

// Entry function: strict pass-through (no guards)
int entry_func(AACEncContext *s, ChannelElement *cpe) {
    ff_aac_adjust_common_ltp(s, cpe);
    return 0;
}
