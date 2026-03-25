#include <stddef.h>
#include <string.h>
#include "harness_types.h"
// klee removed for replay
#include <stdlib.h>
#include <stdint.h>

int sepol_user_modify(user_datum_t *usrdatum);

int LLVMFuzzerTestOneInput(const uint8_t *fuzz_data, size_t fuzz_size) {
    if (fuzz_size < 4) return 0;
    // Allocate the object concretely
    user_datum_t *u = (user_datum_t *)calloc(1, sizeof(user_datum_t));
    if (!u) return 0;

    // Make contents symbolic (but keep a real allocation)
    { memcpy(u, fuzz_data + 0, 4); };

    // UAF setup: free the object to create a stale pointer before use
    free(u);

    // Use-after-free: pass stale pointer into the entry/vulnerable function
    sepol_user_modify(u);
    return 0;
}
