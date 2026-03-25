#include <string.h>
// NO_HARNESS_TYPES
// klee removed for replay
#include <stdlib.h>
#include <stddef.h>

// Minimal local types matching harness/node_record.c

typedef struct sepol_handle sepol_handle_t;
typedef struct sepol_node sepol_node_t;

struct sepol_handle {
    void (*msg_callback)(void *arg, const char *fmt, ...);
    void *msg_callback_arg;
    int msg_level;
};

struct sepol_node {
    char *addr;
    size_t addr_sz;
    char *mask;
    size_t mask_sz;
    int proto;
    void *con;
};

int sepol_node_get_mask_bytes(sepol_handle_t *handle, const sepol_node_t *node);

int main() {
    sepol_handle_t *handle = (sepol_handle_t *)calloc(1, sizeof(sepol_handle_t));
    sepol_node_t *node = (sepol_node_t *)calloc(1, sizeof(sepol_node_t));

    { static const unsigned char handle_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(handle, handle_data, (sizeof(*handle) < sizeof(handle_data)) ? sizeof(*handle) : sizeof(handle_data)); };
    { static const unsigned char node_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(node, node_data, (sizeof(*node) < sizeof(node_data)) ? sizeof(*node) : sizeof(node_data)); };

    // Avoid indirect call through ERR macro
    handle->msg_callback = NULL;

    // We take the malloc-fail path, so mask content isn't needed
    size_t msz;
    { static const unsigned char mask_sz_data[] = {0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(&msz, mask_sz_data, (sizeof(msz) < sizeof(mask_sz_data)) ? sizeof(msz) : sizeof(mask_sz_data)); };
    /* klee_assume removed */
    node->mask_sz = msz;

    sepol_node_get_mask_bytes(handle, node);
    return 0;
}
