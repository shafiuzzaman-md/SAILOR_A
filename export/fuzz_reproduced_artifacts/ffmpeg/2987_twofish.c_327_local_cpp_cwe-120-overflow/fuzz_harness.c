#include <stddef.h>
// Combined reproducer for 2987_twofish.c_327_local_cpp_cwe-120-overflow
// Original harness: driver.c + smart_stubs.c + sliced source

// === smart_stubs.c ===
/* Smart stubs — auto-generated from path + vulnerability analysis */
/* Symbolic stubs model the environment: KLEE explores return values */
/* that both REACH the sink AND TRIGGER the vulnerability */
#include <stdlib.h>
#include <string.h>
/* PROACTIVE: twofish_decrypt (auto-detected external) */
int twofish_decrypt() { return 0; }

/* PROACTIVE: twofish_encrypt (auto-detected external) */
int twofish_encrypt() { return 0; }

// === driver.c ===
#include "harness_types.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
// Prototype of the entry function from the harness
extern int entry_func(AVTWOFISH *cs, uint8_t *dst, const uint8_t *src, int count, uint8_t *iv, int decrypt);

int LLVMFuzzerTestOneInput(const uint8_t *fuzz_data, size_t fuzz_size) {
    if (fuzz_size < 64) return 0;
    // Allocate context
    AVTWOFISH *cs = (AVTWOFISH*)calloc(1, sizeof(AVTWOFISH));

    // Allocate buffers
    uint8_t *dst = (uint8_t*)malloc(16);   // at least 16 bytes so reads are in-bounds
    uint8_t *src = (uint8_t*)malloc(16);   // not used in the neutralized path, but allocate anyway

    // Intentionally undersized IV to trigger overflow in memcpy(iv, dst, 16)
    uint8_t *iv = (uint8_t*)malloc(8);     // smaller than 16 to cause OOB write

    // Make buffer contents symbolic
    memcpy(dst, fuzz_data + (0), 16);
    memcpy(src, fuzz_data + (16), 16);
    memcpy(iv, fuzz_data + (16 + 16), 8);

    int count = 1;     // neutralized loop; value doesn't matter
    int decrypt = 0;   // go into the encrypt branch where memcpy occurs

    // Ensure iv is non-NULL path is taken (it is non-NULL by construction)
    (void)count; (void)decrypt;  // silence unused if optimized

    // Direct call via entry
    entry_func(cs, dst, src, count, iv, decrypt);

    return 0;
}
