#include "harness_types.h"
#include <klee/klee.h>
#include <stdint.h>
#include <stdlib.h>

// Declaration of entry function from harness
int entry_func(AACEncContext *s, ChannelElement *cpe);

int main() {
    // Allocate concrete objects
    AACEncContext *s = (AACEncContext *)calloc(1, sizeof(AACEncContext));
    ChannelElement *cpe = (ChannelElement *)calloc(1, sizeof(ChannelElement));

    // Ensure the condition evaluates the window_sequence[0] reads
    cpe->common_window = 1;  // so !common_window is false, forcing evaluation of next terms

    // Setup sce0 and sce1
    SingleChannelElement *sce0 = &cpe->ch[0];
    SingleChannelElement *sce1 = &cpe->ch[1];

    // Make sce0->ics.window_sequence invalid (NULL) to trigger OOB/invalid dereference on [0]
    sce0->ics.window_sequence = (uint8_t *)0;

    // For sce1, give a small valid buffer so evaluation can proceed if needed
    uint8_t *buf1 = (uint8_t *)malloc(1);
    if (buf1) {
        klee_make_symbolic(buf1, 1, "sce1_window_seq");
        sce1->ics.window_sequence = buf1;
    }

    // Call entry
    entry_func(s, cpe);
    return 0;
}
