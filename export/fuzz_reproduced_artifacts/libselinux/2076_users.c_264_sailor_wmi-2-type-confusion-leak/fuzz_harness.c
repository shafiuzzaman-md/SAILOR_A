#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "harness_types.h"
#include <stdlib.h>
// klee removed for replay

// entry_func is defined in harness/users.c
int sepol_user_modify(sepol_handle *handle);

int LLVMFuzzerTestOneInput(const uint8_t *fuzz_data, size_t fuzz_size) {
    if (fuzz_size < 16) return 0;
    // Allocate a real sepol_handle object
    sepol_handle *h = (sepol_handle *)malloc(sizeof(sepol_handle));
    if (!h) return 0;

    // Make its contents symbolic (attacker-controlled)
    { memcpy(h, fuzz_data + 0, 16); };

    // Model the WMI-2 stale reference: free the object but keep a pointer to it
    free(h);

    // Use-after-free: pass the freed pointer to the entry function
    sepol_user_modify(h);
    return 0;
}
