#include <stdint.h>
#include <stddef.h>
// Combined reproducer for 4370_cavs.c_733_local_cpp_cwe-125-cursor-lookahead-missing-bytes-check
// Original harness: driver.c + smart_stubs.c + sliced source

// === smart_stubs.c ===
/* Smart stubs — auto-generated from path + vulnerability analysis */
/* Symbolic stubs model the environment: KLEE explores return values */
/* that both REACH the sink AND TRIGGER the vulnerability */
#include <stdlib.h>
#include <string.h>
/* PROACTIVE: function (auto-detected external) */
int function() { return 0; }

// === driver.c ===
#include "harness_types.h"
#include <stdlib.h>

int ff_cavs_init_pic(AVSContext *h);

int LLVMFuzzerTestOneInput(const uint8_t *fuzz_data, size_t fuzz_size) {
    if (fuzz_size < 64) return 0;
    // Intentionally under-allocate to force OOB at the vulnerable statement
    // AVSContext here contains only: int pred_mode_Y[9];
    // Allocate too few bytes so h->pred_mode_Y[3] write goes out-of-bounds
    size_t tiny = 8;  // concrete small size
    AVSContext *h = (AVSContext *)calloc(1, tiny);
    if (!h) return 0;

    // Optionally make the tiny region symbolic; not required for crash
    memcpy(h, fuzz_data + (0), tiny);

    // Direct call to the vulnerable function
    ff_cavs_init_pic(h);
    return 0;
}
