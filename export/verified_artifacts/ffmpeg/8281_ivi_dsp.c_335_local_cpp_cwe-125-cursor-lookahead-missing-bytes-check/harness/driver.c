#include "harness_types.h"
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <klee/klee.h>

// Forward declaration of the entry function implemented in harness/ivi_dsp.c
int entry_func(const int32_t *in, int16_t *out, ptrdiff_t pitch, const uint8_t *flags);

int main() {
    // Allocate a deliberately small input buffer to trigger OOB read of in[0..7]
    size_t in_elems = 4;  // fewer than 8 elements needed per row
    int32_t *in_buf = (int32_t *)malloc(in_elems * sizeof(int32_t));
    if (!in_buf) return 0;
    klee_make_symbolic(in_buf, in_elems * sizeof(int32_t), "in_buf");

    // Allocate an output buffer with enough space for up to 8 rows of 8 samples
    size_t out_elems = 8 * 8;  // 64 int16_t elements
    int16_t *out_buf = (int16_t *)malloc(out_elems * sizeof(int16_t));
    if (!out_buf) return 0;

    // flags are unused by the function but pass a valid pointer
    uint8_t *flags = (uint8_t *)malloc(1);
    if (!flags) return 0;
    klee_make_symbolic(flags, 1, "flags");

    // Set pitch so that out += pitch stays within out_buf when explored
    ptrdiff_t pitch = 8;  // 8 int16_t per row

    // Call the entry (pass-through) to the vulnerable function
    entry_func((const int32_t *)in_buf, out_buf, pitch, (const uint8_t *)flags);
    return 0;
}
