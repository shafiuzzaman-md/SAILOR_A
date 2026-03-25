#include <stdint.h>
#include <stddef.h>
#include "harness_types.h"
// klee removed for replay
#include <stdlib.h>
#include <string.h>

// Declarations from the harness
extern int sepol_user_key_unpack(sepol_user_key_t *key);
extern void sepol_user_key_free(sepol_user_key_t *key);

int LLVMFuzzerTestOneInput(const uint8_t *fuzz_data, size_t fuzz_size) {
    if (fuzz_size < 32) return 0;
    // Phase 1: allocate a real key object and its name buffer
    sepol_user_key_t *key = (sepol_user_key_t *)malloc(sizeof(sepol_user_key_t));
    if (!key) return 0;

    const size_t name_sz = 32; // concrete size as required
    char *name_buf = (char *)malloc(name_sz);
    if (!name_buf) return 0;
    { memcpy(name_buf, fuzz_data + 0, 32); };
    // ensure it is a valid C-string (optional, not required for the bug)
    name_buf[name_sz - 1] = '\0';

    key->name = name_buf;

    // Phase 2: free the object to create a stale pointer for UAF/Type-Confusion path
    sepol_user_key_free(key);

    // Phase 3: use-after-free via stale pointer in vulnerable function
    sepol_user_key_unpack(key);

    return 0;
}
