#include "harness_types.h"
// klee removed
#include <stdlib.h>
#include <string.h>

/* Simple stubs that take the success path (no conflicting assumes) */
int node_alloc_addr_string(sepol_handle_t *handle, int proto, char **out) {
    (void)handle; (void)proto;
    size_t sz = 64; // fixed buffer size
    char *buf = (char *)malloc(sz);
    if (!buf) return -1;
    memset(buf, sz, "tmp_mask_buf") /* stub */;;
    buf[sz-1] = '\0';
    *out = buf;
    return 0; // succeed
}

int node_expand_addr(sepol_handle_t *handle, const char *mask, int proto, char *out) {
    (void)handle; (void)proto;
    if (mask && out) {
        // Bounded copy/mix to keep within stub buffer size
        for (size_t i = 0; i < 8; ++i) out[i] = (mask[i % 4] ^ 0x5A);
    }
    return 0; // succeed
}
