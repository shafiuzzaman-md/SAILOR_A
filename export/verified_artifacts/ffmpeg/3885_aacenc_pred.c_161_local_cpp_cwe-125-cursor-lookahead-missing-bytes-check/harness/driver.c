#include "harness_types.h"
#include <stdint.h>
#include <stdlib.h>
#include <klee/klee.h>

// entry_func is defined in the harness
int entry_func(AACEncContext *s, ChannelElement *cpe);

int main(void) {
    // Allocate contexts concretely
    AACEncContext *s = (AACEncContext *)calloc(1, sizeof(*s));
    ChannelElement *cpe = (ChannelElement *)calloc(1, sizeof(*cpe));

    // Satisfy guard: !cpe->common_window must be false so that next OR term evaluates
    cpe->common_window = 1;

    // Prepare window_sequence pointers
    // sce0: zero-sized allocation to trigger OOB on index [0]
    uint8_t *wseq0 = (uint8_t *)malloc(0);
    cpe->ch[0].ics.window_sequence = wseq0;

    // sce1: allocate at least 1 byte; make content symbolic (not required for the crash)
    uint8_t *wseq1 = (uint8_t *)malloc(1);
    klee_make_symbolic(wseq1, 1, "wseq1");
    cpe->ch[1].ics.window_sequence = wseq1;

    // Invoke entry (pass-through to vulnerable function)
    entry_func(s, cpe);
    return 0;
}
