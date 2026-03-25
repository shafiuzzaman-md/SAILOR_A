#include <stdint.h>
#include <stddef.h>
#include "harness_types.h"
// klee removed for replay
#include <stdlib.h>
#include <string.h>

#ifndef NAME_BUF_SZ
#define NAME_BUF_SZ 32
#endif

int js_delglobal(js_State *J, const char *name);

int LLVMFuzzerTestOneInput(const uint8_t *fuzz_data, size_t fuzz_size) {
    if (fuzz_size < 36) return 0;
    // Allocate state and global object concretely
    js_State *J = (js_State *)calloc(1, sizeof(js_State));
    js_Object *G = (js_Object *)calloc(1, sizeof(js_Object));

    // Make contents symbolic (addresses remain concrete)
    { memcpy(G, fuzz_data + 0, 4); };

    // Link and then free to create a dangling pointer (UAF setup)
    J->G = G;
    free(G); // J->G is now a stale pointer

    // Prepare name buffer (symbolic content, NUL-terminated)
    char *name = (char *)calloc(NAME_BUF_SZ, 1);
    { memcpy(name, fuzz_data + 4, 32); };
    name[NAME_BUF_SZ - 1] = '\0';

    // Directly call entry to hit the vulnerable statement
    js_delglobal(J, (const char *)name);
    return 0;
}
