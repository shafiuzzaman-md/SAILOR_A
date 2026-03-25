#include <stddef.h>
#include "harness_types.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
// klee removed for replay

// Forward decl for entry in harness
int policydb_to_image(sepol_handle_t *handle, policydb_t *policydb, void **newdata, size_t *newlen);

int LLVMFuzzerTestOneInput(const uint8_t *fuzz_data, size_t fuzz_size) {
    if (fuzz_size < 60) return 0;
    // Allocate real objects with concrete sizes
    sepol_handle_t *handle = (sepol_handle_t *)malloc(sizeof(sepol_handle_t));
    policydb_t *policydb = (policydb_t *)malloc(sizeof(policydb_t));
    void *newdata = NULL;
    size_t newlen = 0;

    // Make handle contents symbolic (attacker-controlled), but pointer itself is concrete
    { memcpy(handle, fuzz_data + 0, 56); };
    { memcpy(policydb, fuzz_data + 56, 4); };

    // Free the handle to create a stale pointer (UAF setup)
    sepol_handle_destroy(handle);

    // Call entry with the stale pointer; ERR(handle, ...) will dereference freed memory
    (void)policydb_to_image(handle, policydb, &newdata, &newlen);

    return 0;
}
