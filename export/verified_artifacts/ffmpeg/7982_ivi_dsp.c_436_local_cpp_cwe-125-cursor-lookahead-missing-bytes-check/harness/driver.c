// NO_HARNESS_TYPES
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <klee/klee.h>

// entry prototype from harness
int ivi_entry(const int32_t *in, int16_t *out, ptrdiff_t pitch, const uint8_t *flags);

int main() {
    // Allocate an undersized input buffer: only 3 int32_t elements to trigger in[3]
    int32_t *in = (int32_t *)malloc(3 * sizeof(int32_t));
    if (!in) return 0;
    klee_make_symbolic(in, 3 * sizeof(int32_t), "in_buf");

    // Out buffer: minimal valid region
    int16_t *out = (int16_t *)malloc(4 * sizeof(int16_t));
    if (!out) return 0;
    klee_make_symbolic(out, 4 * sizeof(int16_t), "out_buf");

    // Flags buffer (unused in this path but provide a valid pointer)
    uint8_t *flags = (uint8_t *)malloc(4);
    if (!flags) return 0;
    klee_make_symbolic(flags, 4, "flags_buf");

    // Pitch can be any small positive value
    ptrdiff_t pitch = 4;

    // Call entry which directly invokes the vulnerable function
    ivi_entry(in, out, pitch, flags);
    return 0;
}
