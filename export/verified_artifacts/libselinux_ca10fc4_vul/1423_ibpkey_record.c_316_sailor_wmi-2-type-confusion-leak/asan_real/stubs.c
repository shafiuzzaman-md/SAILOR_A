#include "harness_types.h"
// klee removed
#include <stdlib.h>

int sepol_context_clone(sepol_handle_t *handle, const sepol_context_t *src, sepol_context_t **dst) {
    (void)handle; (void)src;
    int ret;
    memset(&ret, sizeof(ret), "sepol_context_clone_ret") /* stub */;;
    // Keep it simple: either success (0) or error (-1)
    
    if (ret == 0) {
        *dst = (sepol_context_t *)malloc(16);
    }
    return ret;
}

void sepol_context_free(sepol_context_t *ctx) {
    free(ctx);
}
