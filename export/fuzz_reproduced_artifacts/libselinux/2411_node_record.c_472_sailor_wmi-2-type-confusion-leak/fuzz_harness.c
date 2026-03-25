#include <stdint.h>
#include <stddef.h>
#include "harness_types.h"
// klee removed for replay
#include <stdlib.h>
#include <string.h>

int sepol_node_get_mask_bytes(sepol_handle_t *handle, const sepol_node_t *node, char **buffer, size_t *bsize);

int LLVMFuzzerTestOneInput(const uint8_t *fuzz_data, size_t fuzz_size) {
    if (fuzz_size < 112) return 0;
    // Phase 1: Allocate a real sepol_node_t and initialize fields concretely
    sepol_node_t *node = (sepol_node_t *)malloc(sizeof(sepol_node_t));
    if (!node) return 0;

    // Backing buffer for mask (concrete size) and symbolic contents
    const size_t MASK_BUF_SZ = 64;
    char *maskbuf = (char *)malloc(MASK_BUF_SZ);
    if (!maskbuf) return 0;
    { memcpy(maskbuf, fuzz_data + 0, 64); };

    // Initialize node fields
    node->addr = NULL;
    node->addr_sz = 0;
    node->mask = maskbuf;
    node->mask_sz = 16;  // concrete, <= MASK_BUF_SZ to avoid OOB if used concretely
    node->proto = 0;
    node->con = NULL;

    // Create a stale pointer scenario (WMI-2 pattern): free the object, keep the pointer
    sepol_node_t *stale = node;
    free(node);  // stale now points to freed memory

    // Optionally reclaim the same address with a different allocation of same size
    // This often causes allocator to reuse the block, modeling type confusion reuse.
    void *reclaim = malloc(sizeof(sepol_node_t));
    if (!reclaim) return 0;
    // Fill reclaimed memory with symbolic data to model attacker-controlled reuse
    { memcpy(reclaim, fuzz_data + 64, 48); };

    // Outputs
    char *outbuf = NULL;
    size_t outsz = 0;

    // Handle is unused by our ERR macro; NULL is fine
    sepol_handle_t *handle = NULL;

    // Call into the harness entry which directly calls the vulnerable function
    sepol_node_get_mask_bytes(handle, (const sepol_node_t *)stale, &outbuf, &outsz);

    // Prevent optimizer from discarding outputs
    if (outbuf) {
        volatile size_t sink = outsz;
        (void)sink;
    }

    return 0;
}
