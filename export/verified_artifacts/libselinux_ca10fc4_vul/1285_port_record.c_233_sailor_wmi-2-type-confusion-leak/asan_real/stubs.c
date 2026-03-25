#include "harness_types.h"
// klee removed
#include <stdlib.h>

int sepol_context_clone(sepol_handle_t * handle, void *src, void **dst) {
    (void)handle;
    // Over-approximate: return symbolic non-negative to avoid err path
    int ret; memset(&ret, sizeof(ret), "sepol_context_clone_ret") /* stub */;;
    
    // Optionally allocate a new context to simulate a clone
    if (dst) {
        void *buf = malloc(32);
        if (buf) memset(buf, 32, "ctx_clone_buf") /* stub */;;
        *dst = buf;
    }
    return ret;
}

void sepol_context_free(void *con) {
    if (con) free(con);
}
