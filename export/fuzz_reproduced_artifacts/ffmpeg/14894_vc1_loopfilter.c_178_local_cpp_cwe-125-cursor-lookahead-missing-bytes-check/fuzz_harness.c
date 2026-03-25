#include <stddef.h>
// Combined reproducer for 14894_vc1_loopfilter.c_178_local_cpp_cwe-125-cursor-lookahead-missing-bytes-check
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
#include <stdint.h>
#include <string.h>
// Declaration from harness
int harness_entry(VC1Context *v);

int LLVMFuzzerTestOneInput(const uint8_t *fuzz_data, size_t fuzz_size) {
    if (fuzz_size < 64) return 0;
    VC1Context *v = (VC1Context *)calloc(1, sizeof(VC1Context));
    if (!v) return 0;

    // Allocate block_index with at least 1 element and set it to 0 so (idx - 1) becomes -1
    int *block_index = (int *)malloc(sizeof(int) * 1);
    if (!block_index) return 0;
    block_index[0] = 0;
    v->s.block_index = block_index;

    // Allocate mb_type[0] with a small concrete size (the -1 access will be OOB)
    v->mb_type[0] = (uint8_t *)malloc(4);
    if (!v->mb_type[0]) return 0;
    // Make contents symbolic but force element 0 to be non-zero so the second operand is evaluated
    memcpy(v->mb_type[0], fuzz_data + (0), 4);
    v->mb_type[0][0] = 1;

    // Call entry (pure pass-through to vulnerable function)
    int ret = harness_entry(v);
    return ret;
}
