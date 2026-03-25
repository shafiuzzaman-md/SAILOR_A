#include "harness_types.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
// klee removed for replay

// Forward decl for entry in harness
int policydb_to_image(sepol_handle_t *handle, policydb_t *policydb, void **newdata, size_t *newlen);

int main() {
    // Allocate real objects with concrete sizes
    sepol_handle_t *handle = (sepol_handle_t *)malloc(sizeof(sepol_handle_t));
    policydb_t *policydb = (policydb_t *)malloc(sizeof(policydb_t));
    void *newdata = NULL;
    size_t newlen = 0;

    // Make handle contents symbolic (attacker-controlled), but pointer itself is concrete
    { static const unsigned char handle_bytes_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(handle, handle_bytes_data, (sizeof(*handle) < sizeof(handle_bytes_data)) ? sizeof(*handle) : sizeof(handle_bytes_data)); };
    { static const unsigned char policydb_bytes_data[] = {0x00, 0x00, 0x00, 0x00}; memcpy(policydb, policydb_bytes_data, (sizeof(*policydb) < sizeof(policydb_bytes_data)) ? sizeof(*policydb) : sizeof(policydb_bytes_data)); };

    // Free the handle to create a stale pointer (UAF setup)
    sepol_handle_destroy(handle);

    // Call entry with the stale pointer; ERR(handle, ...) will dereference freed memory
    (void)policydb_to_image(handle, policydb, &newdata, &newlen);

    return 0;
}
