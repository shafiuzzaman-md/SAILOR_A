#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <klee/klee.h>

// Entry function must be a direct pass-through to the vulnerable function
int harness_entry(const int32_t *in, int16_t *out, ptrdiff_t pitch, const uint8_t *flags);
void ff_ivi_row_slant4(const int32_t *in, int16_t *out, ptrdiff_t pitch, const uint8_t *flags);

int harness_entry(const int32_t *in, int16_t *out, ptrdiff_t pitch, const uint8_t *flags) {
    ff_ivi_row_slant4(in, out, pitch, flags);  // DIRECT call
    return 0;
}

// Neutralized vulnerable function keeping the exact vulnerable statement
void ff_ivi_row_slant4(const int32_t *in, int16_t *out, ptrdiff_t pitch, const uint8_t *flags)
{
    int t0, t1, t2, t3, t4;  // kept from original decls (unused in this slice)

#define COMPENSATE(x) (((x) + 1)>>1)
    // EXACT vulnerable line from ivi_dsp.c:716
    if (!in[0] && !in[1] && !in[2] && !in[3]) {
        // body sliced away — not needed for triggering the lookahead read
    } else {
        ; // sliced: avoid pulling heavy macros
    }
#undef COMPENSATE

    // Universal sink assertion placed AFTER the vulnerable statement
    klee_assert(0 && "SAILOR_SINK_REACHED");
}
