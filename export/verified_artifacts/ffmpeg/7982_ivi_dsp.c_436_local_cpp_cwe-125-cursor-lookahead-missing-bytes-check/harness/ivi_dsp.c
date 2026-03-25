#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <klee/klee.h>

// Neutralized vulnerable function: keep the exact vulnerable statement
void ff_ivi_row_haar4(const int32_t *in, int16_t *out, ptrdiff_t pitch,
                      const uint8_t *flags)
{
    // Vulnerable lookahead read (must be verbatim):
    if (!in[0] && !in[1] && !in[2] && !in[3]) {
        // neutralized body
    } else {
        // neutralized body
    }
    // Universal sink assertion placed AFTER the vulnerable statement
    klee_assert(0 && "SAILOR_SINK_REACHED");
}

// Simple pass-through entry function (no guards)
int ivi_entry(const int32_t *in, int16_t *out, ptrdiff_t pitch, const uint8_t *flags) {
    ff_ivi_row_haar4(in, out, pitch, flags);
    return 0;
}
