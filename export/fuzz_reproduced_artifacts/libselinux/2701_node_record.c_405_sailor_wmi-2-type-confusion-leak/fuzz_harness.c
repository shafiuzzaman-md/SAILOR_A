#include <stdint.h>
// NO_HARNESS_TYPES
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
// klee removed for replay

/* Minimal replicas matching harness definitions */
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

int sepol_node_set_addr(sepol_handle_t *handle, sepol_node_t *node, int proto, const char *addr);

int LLVMFuzzerTestOneInput(const uint8_t *fuzz_data, size_t fuzz_size) {
    if (fuzz_size < 8) return 0;
    sepol_handle_t *handle = (sepol_handle_t *)calloc(1, sizeof(sepol_handle_t));
    sepol_node_t *node = (sepol_node_t *)calloc(1, sizeof(sepol_node_t));

    char *base = (char *)malloc(8);
    { memcpy(base, fuzz_data + 0, 8); };
    node->addr = base + 1;  // interior pointer so free(node->addr) is invalid
    node->addr_sz = 7;
    node->mask = NULL;
    node->mask_sz = 0;
    node->proto = 4;
    node->con = NULL;

    const char *addr_str = "1.2.3.4";

    sepol_node_set_addr(handle, node, node->proto, addr_str);
    return 0;
}
