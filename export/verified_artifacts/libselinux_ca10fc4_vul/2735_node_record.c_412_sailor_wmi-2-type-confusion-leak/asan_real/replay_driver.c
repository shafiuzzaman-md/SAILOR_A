// NO_HARNESS_TYPES
// klee removed for replay
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

// Minimal replicas matching harness signatures
typedef void (*sepol_msg_callback_t)(void *arg, const char *fmt, ...);
typedef struct sepol_handle {
    sepol_msg_callback_t msg_callback;
    void *msg_callback_arg;
} sepol_handle_t;

typedef struct sepol_node {
    char *addr;
    size_t addr_sz;
    int proto;
} sepol_node_t;

int sepol_node_set_addr(sepol_handle_t *handle, sepol_node_t *node, int proto, const char *addr);

static void dummy_cb(void *arg, const char *fmt, ...) {
    (void)arg; (void)fmt;  // no-op
}

int main() {
    sepol_handle_t *handle = (sepol_handle_t *)calloc(1, sizeof(sepol_handle_t));
    if (!handle) return 0;
    handle->msg_callback = dummy_cb;
    handle->msg_callback_arg = calloc(1, 16);

    // Free the handle to create a stale pointer (UAF / type confusion scenario)
    free(handle);

    // Reclaim the freed slot with attacker-controlled bytes
    void *reclaim = malloc(sizeof(sepol_handle_t));
    if (reclaim) {
        { static const unsigned char reclaim_blob_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(reclaim, reclaim_blob_data, (sizeof(sepol_handle_t) < sizeof(reclaim_blob_data)) ? sizeof(sepol_handle_t) : sizeof(reclaim_blob_data)); };
    }

    sepol_node_t *node = (sepol_node_t *)calloc(1, sizeof(sepol_node_t));
    if (!node) return 0;
    node->addr = (char *)malloc(8);
    node->addr_sz = 8;
    node->proto = 0;

    char *addr = (char *)malloc(32);
    { static const unsigned char addr_str_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(addr, addr_str_data, (32 < sizeof(addr_str_data)) ? 32 : sizeof(addr_str_data)); };
    addr[31] = '\0';

    int proto;
    { static const unsigned char proto_data[] = {0x00, 0x00, 0x00, 0x00}; memcpy(&proto, proto_data, (sizeof(proto) < sizeof(proto_data)) ? sizeof(proto) : sizeof(proto_data)); };

    sepol_node_set_addr(handle, node, proto, addr);
    return 0;
}
