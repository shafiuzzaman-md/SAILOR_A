#include "harness_types.h"
// klee removed for replay
#include <stdlib.h>
#include <string.h>

/* Prototypes from harness */
void js_unref(js_State *J, const char *ref);

int main() {
    // Allocate js_State and registry object concretely
    js_State *J = (js_State *)calloc(1, sizeof(js_State));
    if (!J) return 0;
    js_Object *reg = (js_Object *)calloc(1, sizeof(js_Object));
    if (!reg) return 0;

    // Install registry into state (keep it valid — do NOT free here)
    J->R = reg;

    // Large symbolic name intended to stress string operations in jsrun.c
    const size_t N = 512;  // concrete size
    char *name = (char *)malloc(N);
    if (!name) return 0;
    { static const unsigned char prop_name_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(name, prop_name_data, (N < sizeof(prop_name_data)) ? N : sizeof(prop_name_data)); };

    // Encourage overflow: no NUL in the first N-1 bytes
    for (size_t i = 0; i + 1 < N; ++i) {
        /* klee_assume removed */
    }
    name[N - 1] = '\0';  // ensure it's a C-string

    // Call entry that reaches vulnerable site
    js_unref(J, name);
    return 0;
}
