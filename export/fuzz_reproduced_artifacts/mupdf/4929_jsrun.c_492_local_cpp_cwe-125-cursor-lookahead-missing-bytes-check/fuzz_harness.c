#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "harness_types.h"
// klee removed for replay
#include <stdlib.h>

int js_isarrayindex(js_State *J, const char *p, int *idx);

int LLVMFuzzerTestOneInput(const uint8_t *fuzz_data, size_t fuzz_size) {
    if (fuzz_size < 5) return 0;
    js_State *J = NULL; // not used by js_isarrayindex

    // Allocate a 1-byte buffer so that p[1] is out-of-bounds
    char *p = (char *)malloc(1);
    if (!p) return 0;
    { memcpy(p, fuzz_data + 0, 1); };

    // Force the path: p[0] != 0 and p[0] == '0' to hit the vulnerable line accessing p[1]
    /* klee_assume removed */

    int *idx = (int *)malloc(sizeof(int));
    if (!idx) return 0;
    { memcpy(idx, fuzz_data + 1, 4); };

    (void)js_isarrayindex(J, p, idx);
    return 0;
}
