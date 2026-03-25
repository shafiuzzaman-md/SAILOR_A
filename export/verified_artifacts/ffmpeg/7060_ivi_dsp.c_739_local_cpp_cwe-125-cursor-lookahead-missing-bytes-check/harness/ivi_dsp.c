#include <stdint.h>
#include <stddef.h>
#include <klee/klee.h>

// Minimal stub for the transform. Using a real function (not macro) ensures
// that arguments (including in[8]) are evaluated at the call site.
void IVI_INV_SLANT4(int32_t a, int32_t b, int32_t c, int32_t d,
                    int16_t e, int16_t f, int16_t g, int16_t h,
                    int i, int j, int k, int l, int m) {
    (void)a; (void)b; (void)c; (void)d;
    (void)e; (void)f; (void)g; (void)h;
    (void)i; (void)j; (void)k; (void)l; (void)m;
}

// Vulnerable function (neutralized): keep only the path containing the vulnerable statement.
void ff_ivi_col_slant4(const int32_t *in, int16_t *out, ptrdiff_t pitch, const uint8_t *flags) {
    int     t0, t1, t2, t3, t4;
    int     row2;
    row2 = pitch << 1;

    if (1) { // was: for (i = 0; i < 4; i++)
        if (flags[0]) { // was: if (flags[i])
            // Vulnerable statement — must be verbatim from source_context
            IVI_INV_SLANT4(in[0], in[4], in[8], in[12],
                           out[0], out[pitch], out[row2], out[row2 + pitch],
                           t0, t1, t2, t3, t4);
            // Universal sink assertion (after the statement)
            klee_assert(0 && "SAILOR_SINK_REACHED");
        } else {
            out[0] = out[pitch] = out[row2] = out[row2 + pitch] = 0;
        }
    }
}

// Entry function: mandatory simple pass-through to the vulnerable function
int entry_func(const int32_t *in, int16_t *out, ptrdiff_t pitch, const uint8_t *flags) {
    ff_ivi_col_slant4(in, out, pitch, flags);
    return 0;
}
