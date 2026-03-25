#include <string.h>
// NO_HARNESS_TYPES
// klee removed for replay
#include <stddef.h>
#include <stdint.h>

// Minimal compatible type defs matching harness expectations
typedef struct sepol_handle {
    void (*msg_callback)(void *arg, const char *fmt, ...);
    void *msg_callback_arg;
    int msg_level;
} sepol_handle_t;

typedef struct sepol_node {
    char *addr; size_t addr_sz;
    char *mask; size_t mask_sz;
    int proto;
    void *con;
} sepol_node_t;

int sepol_node_set_mask_bytes(sepol_handle_t *handle, sepol_node_t *node, const char *mask, size_t mask_sz);

int main() {
    sepol_handle_t handle_obj;
    sepol_node_t node_obj;

    { static const unsigned char handle_obj_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(&handle_obj, handle_obj_data, (sizeof(handle_obj) < sizeof(handle_obj_data)) ? sizeof(handle_obj) : sizeof(handle_obj_data)); };
    // Prevent callback invocation (we only need to reach ERR statement)
    handle_obj.msg_callback = 0;
    handle_obj.msg_callback_arg = 0;

    { static const unsigned char node_obj_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(&node_obj, node_obj_data, (sizeof(node_obj) < sizeof(node_obj_data)) ? sizeof(node_obj) : sizeof(node_obj_data)); };
    node_obj.mask = 0; // free(NULL) safe if ever reached (we exit earlier)

    char mask_buf[32];
    { static const unsigned char mask_buf_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(mask_buf, mask_buf_data, (sizeof(mask_buf) < sizeof(mask_buf_data)) ? sizeof(mask_buf) : sizeof(mask_buf_data)); };

    size_t mask_sz;
    { static const unsigned char mask_sz_data[] = {0x7f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(&mask_sz, mask_sz_data, (sizeof(mask_sz) < sizeof(mask_sz_data)) ? sizeof(mask_sz) : sizeof(mask_sz_data)); };
    /* klee_assume removed */
    /* klee_assume removed */

    sepol_node_set_mask_bytes(&handle_obj, &node_obj, mask_buf, mask_sz);
    return 0;
}
