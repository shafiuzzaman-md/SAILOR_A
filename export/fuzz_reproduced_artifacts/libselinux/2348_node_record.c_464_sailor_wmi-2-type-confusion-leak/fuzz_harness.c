#include <string.h>
// NO_HARNESS_TYPES
// klee removed for replay
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>

// Mirror minimal types from harness preamble
typedef struct sepol_handle { int dummy; } sepol_handle_t;
typedef struct sepol_context { int dummy; } sepol_context_t;

typedef struct sepol_node {
    char *addr;
    size_t addr_sz;
    char *mask;
    size_t mask_sz;
    int proto;
    sepol_context_t *con;
} sepol_node_t;

// Prototype from harness
extern int sepol_node_get_mask_bytes(sepol_handle_t *handle, const sepol_node_t *node);

int LLVMFuzzerTestOneInput(const uint8_t *fuzz_data, size_t fuzz_size) {
    if (fuzz_size < 16) return 0;
    // Allocate handle and node concretely
    sepol_handle_t *handle = (sepol_handle_t *)calloc(1, sizeof(sepol_handle_t));
    sepol_node_t *node = (sepol_node_t *)calloc(1, sizeof(sepol_node_t));

    // Prepare a small source buffer for node->mask
    const size_t src_sz = 16;  // small concrete allocation
    char *mask = (char *)malloc(src_sz);
    if (!handle || !node || !mask) return 0;  // keep driver simple

    // Make mask content symbolic
    { memcpy(mask, fuzz_data + 0, 16); };

    // Assign to node
    node->mask = mask;

    // Choose a symbolic mask_sz that exceeds src_sz to trigger OOB read in memcpy
    size_t mask_sz;
    { static const unsigned char mask_sz_data[] = {0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(&mask_sz, mask_sz_data, (sizeof(mask_sz) < sizeof(mask_sz_data)) ? sizeof(mask_sz) : sizeof(mask_sz_data)); };
    // Constrain to a reasonable upper bound so malloc succeeds and KLEE explores
    /* klee_assume removed */
    /* klee_assume removed */
    node->mask_sz = mask_sz;

    // Other fields are irrelevant to this path
    node->addr = NULL;
    node->addr_sz = 0;
    node->proto = 0;
    node->con = NULL;

    // Call entry (pure pass-through to vulnerable function)
    (void)sepol_node_get_mask_bytes(handle, node);

    return 0;
}
