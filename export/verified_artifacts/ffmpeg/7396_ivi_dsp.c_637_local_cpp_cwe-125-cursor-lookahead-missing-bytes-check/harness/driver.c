#include "harness_types.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <klee/klee.h>

int entry_func(const int32_t *in, int16_t *out, ptrdiff_t pitch, const uint8_t *flags);

int main() {
    // Allocate a small input buffer (fewer than 8 int32_t elements) to trigger in[7] OOB read
    const size_t in_elems = 4; // intentionally < 8
    int32_t *in = (int32_t *)malloc(in_elems * sizeof(int32_t));
    klee_make_symbolic(in, in_elems * sizeof(int32_t), "in_buf");

    // Output buffer: 8x8 block with pitch 8 (16 bytes memset per row when if-branch taken)
    const ptrdiff_t pitch = 8;
    const size_t out_elems = 8 * 8; // 64 int16_t elements
    int16_t *out = (int16_t *)calloc(out_elems, sizeof(int16_t));
    klee_make_symbolic(out, out_elems * sizeof(int16_t), "out_buf");

    // Flags are unused in our slice but allocate to be safe
    uint8_t *flags = (uint8_t *)malloc(8);
    klee_make_symbolic(flags, 8, "flags_buf");

    // Direct call to entry function (pass-through to ff_ivi_row_slant8)
    entry_func(in, out, pitch, flags);

    return 0;
}
