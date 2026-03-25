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
