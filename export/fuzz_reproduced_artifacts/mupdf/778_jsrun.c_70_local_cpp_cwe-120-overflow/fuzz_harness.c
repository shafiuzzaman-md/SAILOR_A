#include <stdint.h>
#include <stddef.h>
#include "harness_types.h"
// klee removed for replay
#include <stdlib.h>
#include <string.h>

// entry from harness
int jsV_newmemstring(struct js_State *J, const char *s, int n);

int LLVMFuzzerTestOneInput(const uint8_t *fuzz_data, size_t fuzz_size) {
    if (fuzz_size < 8) return 0;
    // Allocate js_State concretely
    struct js_State *J = (struct js_State *)calloc(1, sizeof(struct js_State));

    // Allocate a small concrete source buffer
    const int SRC_SZ = 8; // concrete size per instructions
    char *src = (char *)malloc(SRC_SZ);
    if (!J || !src) return 0;

    // Make source contents symbolic
    { memcpy(src, fuzz_data + 0, 8); };

    // Length n is symbolic but constrained to exceed SRC_SZ to cause over-read
    int n;
    { static const unsigned char n_data[] = {0x09, 0x00, 0x00, 0x00}; memcpy(&n, n_data, (sizeof(n) < sizeof(n_data)) ? sizeof(n) : sizeof(n_data)); };
    /* klee_assume removed */
    /* klee_assume removed */ // keep allocations reasonable

    // Call entry (direct pass-through to vulnerable memcpy)
    jsV_newmemstring(J, src, n);
    return 0;
}
