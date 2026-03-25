// NO_HARNESS_TYPES
// klee removed for replay
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

// Mirror the harness typedefs exactly
#ifndef STATUS_SUCCESS
#define STATUS_SUCCESS 0
#endif
#ifndef STATUS_ERR
#define STATUS_ERR -1
#endif

typedef struct sepol_handle { int dummy; } sepol_handle_t;

typedef struct sepol_node {
    char *addr;
    size_t addr_sz;
    char *mask;
    size_t mask_sz;
    int proto;
} sepol_node_t;

// Prototype from harness (must match exactly)
int sepol_node_key_extract(sepol_handle_t *handle, const sepol_node_t *node);

int main() {
    // Allocate handle concretely
    sepol_handle_t *handle = (sepol_handle_t *)calloc(1, sizeof(sepol_handle_t));

    // Allocate node concretely
    sepol_node_t *node = (sepol_node_t *)calloc(1, sizeof(sepol_node_t));

    // Small concrete buffers for source addr/mask
    const size_t SRC_SMALL = 8;    // small source buffers
    char *addr_buf = (char *)malloc(SRC_SMALL);
    char *mask_buf = (char *)malloc(SRC_SMALL);

    // Make their contents symbolic (bytes), but keep allocation sizes concrete
    { static const unsigned char addr_buf_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(addr_buf, addr_buf_data, (SRC_SMALL < sizeof(addr_buf_data)) ? SRC_SMALL : sizeof(addr_buf_data)); };
    { static const unsigned char mask_buf_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(mask_buf, mask_buf_data, (SRC_SMALL < sizeof(mask_buf_data)) ? SRC_SMALL : sizeof(mask_buf_data)); };

    // Assign to node
    node->addr = addr_buf;
    node->mask = mask_buf;

    // Set sizes LARGER than the allocated source to cause over-read in memcpy
    node->addr_sz = 64;  // larger than SRC_SMALL
    node->mask_sz = 64;  // larger than SRC_SMALL

    // Protocol value irrelevant for this path
    node->proto = 0;

    // Call entry (strict pass-through to vulnerable function)
    sepol_node_key_extract(handle, node);

    return 0;
}
