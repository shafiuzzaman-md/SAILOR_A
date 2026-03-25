#include <stdint.h>
#include <stddef.h>
// Combined reproducer for 3861_aacenc_tns.c_167_local_cpp_cwe-125-cursor-lookahead-missing-bytes-check
// Original harness: driver.c + smart_stubs.c + sliced source

// === smart_stubs.c ===
/* Smart stubs — auto-generated from path + vulnerability analysis */
/* Symbolic stubs model the environment: KLEE explores return values */
/* that both REACH the sink AND TRIGGER the vulnerability */
#include <stdlib.h>
#include <string.h>
/* PROACTIVE: through (auto-detected external) */
int through() { return 0; }

// === driver.c ===
#include "harness_types.h"
#include <stdlib.h>

// entry from harness
int entry_func(AACEncContext *s, SingleChannelElement *sce);

int LLVMFuzzerTestOneInput(const uint8_t *fuzz_data, size_t fuzz_size) {
    if (fuzz_size < 64) return 0;
    // Concrete allocations (no symbolic sizes)
    AACEncContext *s = (AACEncContext *)calloc(1, sizeof(AACEncContext));
    SingleChannelElement *sce = (SingleChannelElement *)calloc(1, sizeof(SingleChannelElement));

    // Make the window_sequence pointer symbolic; it may be NULL or invalid,
    // which will trigger the OOB/invalid read at window_sequence[0]
    int *winseq_ptr;
    memcpy(&winseq_ptr, fuzz_data + (0), sizeof(winseq_ptr));
    sce->ics.window_sequence = winseq_ptr;

    // Call entry directly (no guards)
    entry_func(s, sce);
    return 0;
}
