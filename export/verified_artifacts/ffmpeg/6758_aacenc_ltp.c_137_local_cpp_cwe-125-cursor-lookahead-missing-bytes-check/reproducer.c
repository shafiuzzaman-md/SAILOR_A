// Combined reproducer for 6758_aacenc_ltp.c_137_local_cpp_cwe-125-cursor-lookahead-missing-bytes-check
// Original harness: driver.c + smart_stubs.c + sliced source

// === smart_stubs.c ===
/* Smart stubs — auto-generated from path + vulnerability analysis */
/* Symbolic stubs model the environment: KLEE explores return values */
/* that both REACH the sink AND TRIGGER the vulnerability */
#include <stdlib.h>
#include <string.h>
#include <klee/klee.h>

/* PROACTIVE: function (auto-detected external) */
int function() { return 0; }

/* PROACTIVE: through (auto-detected external) */
int through() { return 0; }

// === driver.c ===
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
