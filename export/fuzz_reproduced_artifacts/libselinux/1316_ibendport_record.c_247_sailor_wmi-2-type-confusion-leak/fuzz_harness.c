#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "harness_types.h"
#include <stdlib.h>
// klee removed for replay

// Prototype from harness
int sepol_ibendport_clone(sepol_handle_t *handle,
                  const sepol_ibendport_t *ibendport,
                  sepol_ibendport_t **ibendport_ptr);

int LLVMFuzzerTestOneInput(const uint8_t *fuzz_data, size_t fuzz_size) {
    if (fuzz_size < 24) return 0;
    // Allocate concrete sepol_handle and make its contents symbolic (not strictly required)
    sepol_handle_t *handle = (sepol_handle_t *)malloc(sizeof(sepol_handle_t));
    if (!handle) return 0;
    { memcpy(handle, fuzz_data + 0, 24); };

    // Prepare other parameters (not used on our neutralized path)
    sepol_ibendport_t *in_ib = (sepol_ibendport_t *)malloc(sizeof(sepol_ibendport_t));
    if (!in_ib) in_ib = NULL;
    sepol_ibendport_t *out_ib = NULL;

    // Free the handle to create a UAF when ERR(handle, ...) dereferences it
    free(handle);

    // Call entry directly; it will immediately go to the ERR path in clone()
    sepol_ibendport_clone(handle, in_ib, &out_ib);
    return 0;
}
