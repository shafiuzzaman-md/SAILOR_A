#include "harness_types.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <klee/klee.h>

// Prototype of the entry function from the harness
extern int entry_func(AVTWOFISH *cs, uint8_t *dst, const uint8_t *src, int count, uint8_t *iv, int decrypt);

int main() {
    // Allocate context
    AVTWOFISH *cs = (AVTWOFISH*)calloc(1, sizeof(AVTWOFISH));

    // Allocate buffers
    uint8_t *dst = (uint8_t*)malloc(16);   // at least 16 bytes so reads are in-bounds
    uint8_t *src = (uint8_t*)malloc(16);   // not used in the neutralized path, but allocate anyway

    // Intentionally undersized IV to trigger overflow in memcpy(iv, dst, 16)
    uint8_t *iv = (uint8_t*)malloc(8);     // smaller than 16 to cause OOB write

    // Make buffer contents symbolic
    klee_make_symbolic(dst, 16, "dst_bytes");
    klee_make_symbolic(src, 16, "src_bytes");
    klee_make_symbolic(iv, 8,  "iv_bytes");

    int count = 1;     // neutralized loop; value doesn't matter
    int decrypt = 0;   // go into the encrypt branch where memcpy occurs

    // Ensure iv is non-NULL path is taken (it is non-NULL by construction)
    (void)count; (void)decrypt;  // silence unused if optimized

    // Direct call via entry
    entry_func(cs, dst, src, count, iv, decrypt);

    return 0;
}
