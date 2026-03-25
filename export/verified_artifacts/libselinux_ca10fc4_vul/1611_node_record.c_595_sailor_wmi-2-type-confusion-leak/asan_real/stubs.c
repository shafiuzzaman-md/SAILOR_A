#include "harness_types.h"
// klee removed
#include <stdlib.h>

int sepol_context_clone(sepol_handle_t *handle, sepol_context_t *src, sepol_context_t **dst) {
    int ret; memset(&ret, sizeof(ret), "sepol_context_clone_ret") /* stub */;;
    // Allocate destination when ret >= 0 to mimic success path shape
    if (ret >= 0) {
        *dst = (sepol_context_t*)malloc(16);
        if (*dst) memset(*dst, 16, "cloned_context") /* stub */;;
    }
    return ret;
}

void sepol_context_free(sepol_context_t *c) {
    free(c);
}
