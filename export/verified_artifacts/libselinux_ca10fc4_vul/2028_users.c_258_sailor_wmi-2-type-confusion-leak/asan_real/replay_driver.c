#include <string.h>
#include "harness_types.h"
#include <stdlib.h>
#include <stdint.h>
// klee removed for replay

// Forward decl from harness
int sepol_user_modify(sepol_handle_t *handle, policydb_t *policydb, const void *key, sepol_user_t *user);

int main() {
    // Opaque handles: allocate raw memory and cast
    void *handle_mem = malloc(64);
    sepol_handle_t *handle = (sepol_handle_t *)handle_mem;
    void *user_mem = malloc(64);
    sepol_user_t *user = (sepol_user_t *)user_mem;

    // Allocate policydb concretely
    policydb_t *policydb = (policydb_t *)calloc(1, sizeof(policydb_t));

    // Allocate reverse map array with a concrete size
    const size_t N = 8; // reasonable small size
    policydb->user_val_to_struct = (user_datum_t **)calloc(N, sizeof(user_datum_t *));

    // Optionally make the table symbolic to explore aliasing, but keep pointer valid
    { static const unsigned char user_val_to_struct_array_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(policydb->user_val_to_struct, user_val_to_struct_array_data, (N * sizeof(user_datum_t *) < sizeof(user_val_to_struct_array_data)) ? N * sizeof(user_datum_t *) : sizeof(user_val_to_struct_array_data)); };

    // Key can be NULL; the harness path does not use it
    const void *key = NULL;

    // Call entry (must be direct pass-through to vulnerable function)
    sepol_user_modify(handle, policydb, key, user);
    return 0;
}
