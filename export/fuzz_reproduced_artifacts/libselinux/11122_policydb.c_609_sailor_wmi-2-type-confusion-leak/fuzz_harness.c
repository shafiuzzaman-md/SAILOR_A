#include <stddef.h>
#include <string.h>
#include "harness_types.h"
// klee removed for replay
#include <stdlib.h>
#include <stdint.h>

// entry_func is defined in harness/policydb.c
int type_datum_init(type_datum_t *x);

int LLVMFuzzerTestOneInput(const uint8_t *fuzz_data, size_t fuzz_size) {
    if (fuzz_size < 16) return 0;
    // Allocate the target object concretely
    type_datum_t *x = (type_datum_t *)malloc(sizeof(type_datum_t));
    if (!x) return 0;

    // Optional: make contents symbolic (not strictly needed for UAF)
    { memcpy(x, fuzz_data + 0, 16); };

    // Phase 1: Free the object (creates stale reference)
    free(x);

    // Phase 2: Use-after-free in type_datum_init via ebitmap_init(&x->types)
    // KLEE should detect a .free.err when writing to freed memory
    type_datum_init(x);
    return 0;
}
