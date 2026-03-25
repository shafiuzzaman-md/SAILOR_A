#include <stddef.h>
// Combined reproducer for 2873_camellia.c_408_local_cpp_cwe-120-overflow
// Original harness: driver.c + smart_stubs.c + sliced source

// === smart_stubs.c ===
/* Smart stubs — auto-generated from path + vulnerability analysis */
/* Symbolic stubs model the environment: KLEE explores return values */
/* that both REACH the sink AND TRIGGER the vulnerability */
#include <stdlib.h>
#include <string.h>
// === driver.c ===
#include "harness_types.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
int entry_func(AVCAMELLIA *cs, uint8_t *dst, const uint8_t *src, int count, uint8_t *iv, int decrypt);

int LLVMFuzzerTestOneInput(const uint8_t *fuzz_data, size_t fuzz_size) {
    if (fuzz_size < 64) return 0;
    // Allocate context
    AVCAMELLIA *cs = (AVCAMELLIA *)calloc(1, sizeof(AVCAMELLIA));
    if (!cs) return 0;
    memcpy(cs, fuzz_data + (0), sizeof(*cs));

    // Allocate buffers
    uint8_t *dst = (uint8_t *)malloc(16);
    uint8_t *src = (uint8_t *)malloc(16);
    // Intentionally small IV to trigger overflow in memcpy(iv, dst, 16)
    uint8_t *iv = (uint8_t *)malloc(8);

    if (!dst || !src || !iv) return 0;

    memcpy(dst, fuzz_data + (sizeof(*cs)), 16);
    memcpy(src, fuzz_data + (sizeof(*cs) + 16), 16);
    memcpy(iv, fuzz_data + (sizeof(*cs) + 16 + 16), 8);

    int count = 1;
    int decrypt = 0; // original vulnerable path is in encrypt branch, but harness always executes memcpy

    // Direct call to entry function
    entry_func(cs, dst, src, count, iv, decrypt);
    return 0;
}
