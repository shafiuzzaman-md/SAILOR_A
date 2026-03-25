#include <stdint.h>
#include <stddef.h>
#include "harness_types.h"
// klee removed for replay
#include <stdlib.h>
#include <string.h>

// Prototype from harness
int sepol_port_key_extract(sepol_handle_t *handle, const sepol_port_t *port, sepol_port_key_t **key_ptr);

int LLVMFuzzerTestOneInput(const uint8_t *fuzz_data, size_t fuzz_size) {
    if (fuzz_size < 48) return 0;
    // Allocate handle concretely
    sepol_handle_t *handle = (sepol_handle_t *)calloc(1, sizeof(sepol_handle_t));

    // Allocate a sepol_port_t, make its contents symbolic
    sepol_port_t *port = (sepol_port_t *)malloc(sizeof(sepol_port_t));
    { memcpy(port, fuzz_data + 0, 24); };

    // Keep a stale pointer to simulate stale reference after free (WMI-2/UAF pattern)
    sepol_port_t *stale = port;

    // Free the original object (do not invalidate 'stale')
    free(port);

    // Encourage allocator reuse with a same-sized allocation of different content
    void *reclaim = malloc(sizeof(sepol_port_t));
    if (reclaim) {
        { memcpy(reclaim, fuzz_data + 24, 24); };
    }

    // Prepare key out-parameter
    sepol_port_key_t *key = NULL;

    // Call directly into the vulnerable function (entry unknown -> direct call)
    (void)sepol_port_key_extract(handle, stale, &key);

    return 0;
}
