// Combined reproducer for 3926_aacenc_pred.c_124_local_cpp_cwe-125-cursor-lookahead-missing-bytes-check
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

// === driver.c ===
#include "harness_types.h"
#include <klee/klee.h>
#include <stdlib.h>
#include <stdint.h>

int harness_entry(AACEncContext *s, SingleChannelElement *sce);

int main() {
    AACEncContext *s = (AACEncContext*)calloc(1, sizeof(AACEncContext));
    SingleChannelElement *sce = (SingleChannelElement*)calloc(1, sizeof(SingleChannelElement));
    if (!s || !sce) return 0;

    // Allocate a 1-int buffer, then point window_sequence one-past the end
    int *buf = (int*)malloc(sizeof(int));
    if (!buf) return 0;
    klee_make_symbolic(buf, sizeof(int), "buf_val");

    // Point to one past the allocated region: dereferencing [0] is OOB
    sce->ics.window_sequence = buf + 1;

    // Call entry (pass-through) to trigger the OOB read at window_sequence[0]
    harness_entry(s, sce);
    return 0;
}
