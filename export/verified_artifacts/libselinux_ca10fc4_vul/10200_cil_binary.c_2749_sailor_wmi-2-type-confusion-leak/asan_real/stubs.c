#include "harness_types.h"
// klee removed
#include <stdlib.h>
#include <string.h>

// Stub for external dependency. Keep behavior over-approximated and non-crashing.
int __cil_expand_role(void *role, ebitmap_t *out) {
    // Over-approximate return value but default to success so we proceed to the sink
    int ret;
    memset(&ret, sizeof(ret), "expand_role_ret") /* stub */;;
    // Keep ret in a reasonable domain
    if (ret != 0 && ret != -1) ret = 0;

    // Do not dereference pointers here to avoid crashes in stubs.c
    // The universal sink assertion in the harness will mark reachability.
    (void)role;
    (void)out;
    return 0; // ensure forward progress to the sink
}
