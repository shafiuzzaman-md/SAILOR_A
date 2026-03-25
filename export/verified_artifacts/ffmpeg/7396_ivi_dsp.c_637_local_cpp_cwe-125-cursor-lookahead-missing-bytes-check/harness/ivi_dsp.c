#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <klee/klee.h>

// Neutralized entry: direct pass-through to the vulnerable function
int entry_func(const int32_t *in, int16_t *out, ptrdiff_t pitch, const uint8_t *flags);
void ff_ivi_row_slant8(const int32_t *in, int16_t *out, ptrdiff_t pitch, const uint8_t *flags);

int entry_func(const int32_t *in, int16_t *out, ptrdiff_t pitch, const uint8_t *flags) {
    ff_ivi_row_slant8(in, out, pitch, flags);  // DIRECT call, no guards
    return 0;
}

// Sliced vulnerable function: keep signature and the vulnerable statement verbatim.
// Remove non-essential macros/branches; retain memset in the then-branch (string.h included).
void ff_ivi_row_slant8(const int32_t *in, int16_t *out, ptrdiff_t pitch, const uint8_t *flags)
{
    int     i;
    int     t0, t1, t2, t3, t4, t5, t6, t7, t8;

    for (i = 0; i < 8; i++) {
        if (!in[0] && !in[1] && !in[2] && !in[3] && !in[4] && !in[5] && !in[6] && !in[7]) {
            memset(out, 0, 8*sizeof(out[0]));
        } else {
            // Neutralized: original IVI_INV_SLANT8(...) removed to avoid external deps
            // (not needed to trigger/observe the vulnerable read of in[7]).
        }
        // Reachability probe placed after the vulnerable read expression
        klee_assert(0 && "SAILOR_SINK_REACHED");
        in += 8;
        out += pitch;
    }
}
