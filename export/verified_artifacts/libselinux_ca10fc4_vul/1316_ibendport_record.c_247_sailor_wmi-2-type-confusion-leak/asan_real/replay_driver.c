#include <string.h>
#include "harness_types.h"
#include <stdlib.h>
// klee removed for replay

// Prototype from harness
int sepol_ibendport_clone(sepol_handle_t *handle,
                  const sepol_ibendport_t *ibendport,
                  sepol_ibendport_t **ibendport_ptr);

int main() {
    // Allocate concrete sepol_handle and make its contents symbolic (not strictly required)
    sepol_handle_t *handle = (sepol_handle_t *)malloc(sizeof(sepol_handle_t));
    if (!handle) return 0;
    { static const unsigned char handle_sym_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(handle, handle_sym_data, (sizeof(*handle) < sizeof(handle_sym_data)) ? sizeof(*handle) : sizeof(handle_sym_data)); };

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
