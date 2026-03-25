// NO_HARNESS_TYPES
// klee removed for replay
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>

typedef struct sepol_context sepol_context_t;

typedef struct sepol_handle { int dummy; } sepol_handle_t;

typedef struct sepol_node {
    char *addr;
    size_t addr_sz;
    char *mask;
    size_t mask_sz;
    int proto;
    sepol_context_t *con;
} sepol_node_t;

// entry_func is defined in harness/node_record.c
int sepol_node_get_mask_bytes(sepol_handle_t *handle, const sepol_node_t *node, char **buffer, size_t *bsize);

// klee removed for replay
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

// entry_func is defined in harness/node_record.c
int sepol_node_get_mask_bytes(struct sepol_handle *handle, const struct sepol_node *node, char **buffer, size_t *bsize);

int main() {
    // Allocate handle and node
    struct sepol_handle *handle = (struct sepol_handle *)calloc(1, sizeof(struct sepol_handle));
    struct sepol_node *node = (struct sepol_node *)calloc(1, sizeof(struct sepol_node));

    // Prepare a small concrete mask buffer
    const size_t mask_cap = 8;  // small source buffer
    char *mask = (char *)malloc(mask_cap);
    if (!handle || !node || !mask) return 0;

    // Make contents of mask symbolic
    { static const unsigned char node_mask_bytes_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(mask, node_mask_bytes_data, (mask_cap < sizeof(node_mask_bytes_data)) ? mask_cap : sizeof(node_mask_bytes_data)); };

    // Set node fields
    node->mask = mask;
    node->mask_sz = 0;  // will set via symbolic

    // Make mask_sz symbolic and constrain it to be larger than mask_cap
    size_t msz;
    { static const unsigned char node_mask_sz_data[] = {0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(&msz, node_mask_sz_data, (sizeof(msz) < sizeof(node_mask_sz_data)) ? sizeof(msz) : sizeof(node_mask_sz_data)); };
    /* klee_assume removed */
    /* klee_assume removed */   // reasonable upper bound to keep malloc bounded
    node->mask_sz = msz;

    // Unused fields can remain NULL/0
    node->addr = NULL;
    node->addr_sz = 0;
    node->proto = 0;
    node->con = NULL;

    // Out parameters
    char *out_buf = NULL;
    size_t out_sz = 0;

    // Call entry (pass-through to vulnerable function)
    sepol_node_get_mask_bytes(handle, node, &out_buf, &out_sz);

    return 0;
}
