#include "harness_types.h"
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <klee/klee.h>

int harness_entry(const int32_t *in, int16_t *out, ptrdiff_t pitch, const uint8_t *flags);

int main() {
    // Allocate undersized input buffer: only 3 int32_t elements
    int32_t *in = (int32_t *)malloc(3 * sizeof(int32_t));
    if (!in) return 0;
    klee_make_symbolic(in, 3 * sizeof(int32_t), "in_buf");

    // Output buffer and parameters (not used before the sink in our slice)
    int16_t *out = (int16_t *)malloc(4 * sizeof(int16_t));
    if (!out) return 0;
    klee_make_symbolic(out, 4 * sizeof(int16_t), "out_buf");

    ptrdiff_t pitch = 4;  // one row of 4 elements
    uint8_t *flags = (uint8_t *)malloc(4);
    if (!flags) return 0;
    klee_make_symbolic(flags, 4, "flags_buf");

    // Direct call into the harness entry which calls the vulnerable function
    harness_entry(in, out, pitch, flags);
    return 0;
}
