#include <stddef.h>
// Combined reproducer for 2874_cast5.c_491_local_cpp_cwe-120-overflow
// Original harness: driver.c + smart_stubs.c + sliced source

// === smart_stubs.c ===
/* Smart stubs — auto-generated from path + vulnerability analysis */
/* Symbolic stubs model the environment: KLEE explores return values */
/* that both REACH the sink AND TRIGGER the vulnerability */
#include <stdlib.h>
#include <string.h>
/* PROACTIVE: FUNCTION (auto-detected external) */
int FUNCTION() { return 0; }

/* PROACTIVE: declarations (auto-detected external) */
int declarations() { return 0; }

/* PROACTIVE: project (auto-detected external) */
int project() { return 0; }

// === driver.c ===
#include "harness_types.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// entry from harness
int entry_func(AVCAST5* cs, uint8_t* dst, const uint8_t* src, int count, uint8_t *iv, int decrypt);

int LLVMFuzzerTestOneInput(const uint8_t *fuzz_data, size_t fuzz_size) {
    if (fuzz_size < 64) return 0;
    // Allocate context
    AVCAST5 *cs = (AVCAST5*)calloc(1, sizeof(AVCAST5));

    // Allocate buffers (concrete sizes)
    uint8_t *src = (uint8_t*)malloc(8);
    uint8_t *dst = (uint8_t*)malloc(8);
    // Deliberately small IV to exercise overflow/read issues in the target region
    uint8_t *iv  = (uint8_t*)malloc(4);

    // Make buffer contents symbolic
    memcpy(src, fuzz_data + (0), 8);
    memcpy(dst, fuzz_data + (8), 8);
    memcpy(iv, fuzz_data + (8 + 8), 4);

    int count = 1;     // single block
    int decrypt = 0;   // take encrypt path with IV

    // Call entry (pass-through to vulnerable function)
    entry_func(cs, dst, src, count, iv, decrypt);
    return 0;
}
