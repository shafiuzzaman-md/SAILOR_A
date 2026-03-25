#include <stddef.h>
// Combined reproducer for 5460_cavs.c_196_local_cpp_cwe-125-cursor-lookahead-missing-bytes-check
// Original harness: driver.c + smart_stubs.c + sliced source

// === smart_stubs.c ===
/* Smart stubs — auto-generated from path + vulnerability analysis */
/* Symbolic stubs model the environment: KLEE explores return values */
/* that both REACH the sink AND TRIGGER the vulnerability */
#include <stdlib.h>
#include <string.h>
/* PROACTIVE: function (auto-detected external) */
int function() { return 0; }

/* PROACTIVE: statement (auto-detected external) */
int statement() { return 0; }

// === driver.c ===
#include "harness_types.h"
#include <stdint.h>
#include <stdlib.h>

// Entry from harness
extern int decode_entry(AVSContext *h, uint8_t *top, uint8_t **left, int block);

int LLVMFuzzerTestOneInput(const uint8_t *fuzz_data, size_t fuzz_size) {
    if (fuzz_size < 64) return 0;
    // Allocate an opaque context buffer and cast (struct layout not needed in this slice)
    AVSContext *h = (AVSContext *)calloc(1, 1024);

    // Allocate a 1-byte top buffer so that top[1] is out-of-bounds
    uint8_t *top = (uint8_t *)malloc(1);
    if (!top) return 0;
    memcpy(top, fuzz_data + (0), 1);

    // left is not used along this sliced path, but pass a valid pointer-to-pointer
    uint8_t *left_local = NULL;
    uint8_t **left = &left_local;

    // Target switch case: block == 0
    int block = 0;

    // Direct call into the entry (which directly calls the vulnerable function)
    decode_entry(h, top, left, block);

    return 0;
}
