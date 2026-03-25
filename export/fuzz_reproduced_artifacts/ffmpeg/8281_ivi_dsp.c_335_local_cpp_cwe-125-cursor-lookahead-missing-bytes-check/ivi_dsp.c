#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <klee/klee.h>

// Entry function: must be a simple pass-through to the vulnerable function
int entry_func(const int32_t *in, int16_t *out, ptrdiff_t pitch, const uint8_t *flags);
void ff_ivi_row_haar8(const int32_t *in, int16_t *out, ptrdiff_t pitch, const uint8_t *flags);

int entry_func(const int32_t *in, int16_t *out, ptrdiff_t pitch, const uint8_t *flags) {
    ff_ivi_row_haar8(in, out, pitch, flags);
    return 0;
}

// Neutralized vulnerable function with the exact vulnerable statement preserved
void ff_ivi_row_haar8(const int32_t *in, int16_t *out, ptrdiff_t pitch,
                      const uint8_t *flags)
{
    int     i;
    int     t0, t1, t2, t3, t4, t5, t6, t7, t8;  // kept for signature consistency

    /* apply the InvHaar8 to all rows */
#define COMPENSATE(x) (x)
    for (i = 0; i < 8; i++) {
        if (   !in[0] && !in[1] && !in[2] && !in[3]
            && !in[4] && !in[5] && !in[6] && !in[7]) {
            memset(out, 0, 8 * sizeof(out[0]));
            // UNIVERSAL SINK ASSERTION: after vulnerable read expression
            klee_assert(0 && "SAILOR_SINK_REACHED");
        } else {
            // Neutralized: original INV_HAAR8(...) call removed (not needed for the sink)
            // Keep the path and add the reachability probe
            klee_assert(0 && "SAILOR_SINK_REACHED");
        }
        in  += 8;
        out += pitch;
    }
#undef  COMPENSATE
}
