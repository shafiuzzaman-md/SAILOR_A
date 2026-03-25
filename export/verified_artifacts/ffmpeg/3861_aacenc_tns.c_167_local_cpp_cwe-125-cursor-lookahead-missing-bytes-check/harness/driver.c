#include "harness_types.h"
#include <klee/klee.h>
#include <stdlib.h>

// entry from harness
int entry_func(AACEncContext *s, SingleChannelElement *sce);

int main(void) {
    // Concrete allocations (no symbolic sizes)
    AACEncContext *s = (AACEncContext *)calloc(1, sizeof(AACEncContext));
    SingleChannelElement *sce = (SingleChannelElement *)calloc(1, sizeof(SingleChannelElement));

    // Make the window_sequence pointer symbolic; it may be NULL or invalid,
    // which will trigger the OOB/invalid read at window_sequence[0]
    int *winseq_ptr;
    klee_make_symbolic(&winseq_ptr, sizeof(winseq_ptr), "window_sequence_ptr");
    sce->ics.window_sequence = winseq_ptr;

    // Call entry directly (no guards)
    entry_func(s, sce);
    return 0;
}
