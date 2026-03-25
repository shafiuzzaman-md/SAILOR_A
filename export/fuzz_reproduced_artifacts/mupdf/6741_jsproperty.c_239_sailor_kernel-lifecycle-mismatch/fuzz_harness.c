#include <stdint.h>
#include <stddef.h>
#include "harness_types.h"
// klee removed for replay
#include <stdlib.h>
#include <string.h>

extern int jsV_delproperty(js_State *J, js_Object *obj, const char *name);

int LLVMFuzzerTestOneInput(const uint8_t *fuzz_data, size_t fuzz_size) {
    if (fuzz_size < 16) return 0;
    js_State *J = (js_State *)calloc(1, sizeof(js_State));
    js_Object *obj = (js_Object *)calloc(1, sizeof(js_Object));
    if (!J || !obj) return 0;

    // Allocate a property node and set as root
    js_Property *root = (js_Property *)calloc(1, sizeof(js_Property));
    if (!root) return 0;
    // Set right pointer to ensure non-null access pattern
    js_Property *right = (js_Property *)calloc(1, sizeof(js_Property));
    root->right = right;
    obj->properties = root;

    // Free the root BEFORE calling entry to create a UAF when dereferenced in deleteproperty
    free(root);

    // Symbolic name buffer (NUL-terminated)
    char namebuf[16];
    { memcpy(namebuf, fuzz_data + 0, 16); };
    namebuf[15] = '\0';

    jsV_delproperty(J, obj, namebuf);
    return 0;
}
