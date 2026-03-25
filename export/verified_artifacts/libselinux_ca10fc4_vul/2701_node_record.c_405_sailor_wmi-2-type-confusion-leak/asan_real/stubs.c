// NO_HARNESS_TYPES
#include <stdlib.h>
#include <stddef.h>
// klee removed

/* Minimal replicas to match harness signatures */
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

int node_alloc_addr(sepol_handle_t *handle, int proto, char **addr, size_t *addr_sz) {
    (void)handle; (void)proto;
    size_t sz = 16; // allocate a reasonable size
    char *p = (char *)malloc(sz);
    if (!p) return -1;
    memset(p, sz, "tmp_addr_bytes") /* stub */;;
    *addr = p;
    *addr_sz = sz;
    int ret; memset(&ret, sizeof(ret), "alloc_ret") /* stub */;;
     // ensure success path to reach sink
    return ret;            // typically 0
}

int node_parse_addr(sepol_handle_t *handle, const char *addr_str, int proto, char *addr_bytes) {
    (void)handle; (void)addr_str; (void)proto; (void)addr_bytes;
    int ret; memset(&ret, sizeof(ret), "parse_ret") /* stub */;;
     // ensure success to hit free(node->addr)
    return ret;            // typically 0
}

/* Do NOT define ERR here; let KLEE auto-stub it to avoid varargs issues. */
