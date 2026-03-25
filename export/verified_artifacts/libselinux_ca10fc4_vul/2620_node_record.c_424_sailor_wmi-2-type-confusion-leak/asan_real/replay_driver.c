// NO_HARNESS_TYPES
// klee removed for replay
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

// Minimal replicas matching harness/node_record.c
typedef void (*sepol_msg_callback_t)(void *, const char *, ...);

typedef struct sepol_handle {
    sepol_msg_callback_t msg_callback;
    void *msg_callback_arg;
} sepol_handle_t;

typedef struct sepol_node {
    char *addr;
    size_t addr_sz;
    char *mask;
    size_t mask_sz;
    int proto;
    void *con;
} sepol_node_t;

int sepol_node_set_addr_bytes(sepol_handle_t *handle, sepol_node_t *node, const char *addr, size_t addr_sz);

int main() {
    sepol_handle_t *handle = (sepol_handle_t *)calloc(1, sizeof(sepol_handle_t));
    sepol_node_t *node = (sepol_node_t *)calloc(1, sizeof(sepol_node_t));

    node->addr = NULL;  // free(NULL) is safe
    node->addr_sz = 0;
    node->mask = NULL;
    node->mask_sz = 0;
    node->proto = 0;
    node->con = NULL;

    enum { SRC_BUF_SZ = 32 };
    char *src = (char *)malloc(SRC_BUF_SZ);
    { static const unsigned char addr_src_bytes_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(src, addr_src_bytes_data, (SRC_BUF_SZ < sizeof(addr_src_bytes_data)) ? SRC_BUF_SZ : sizeof(addr_src_bytes_data)); };

    size_t addr_sz;
    { static const unsigned char addr_sz_data[] = {0x21, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(&addr_sz, addr_sz_data, (sizeof(addr_sz) < sizeof(addr_sz_data)) ? sizeof(addr_sz) : sizeof(addr_sz_data)); };
    /* klee_assume removed */
    /* klee_assume removed */

    sepol_node_set_addr_bytes(handle, node, (const char *)src, addr_sz);
    return 0;
}
