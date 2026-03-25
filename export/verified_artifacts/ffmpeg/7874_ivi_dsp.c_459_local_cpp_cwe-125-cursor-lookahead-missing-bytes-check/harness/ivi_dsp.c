#include <stdint.h>
#include <stddef.h>
#include <klee/klee.h>

// Minimal local definition to allow compilation; the vulnerable read happens
// during argument evaluation of the INV_HAAR4 call below.
#ifndef INV_HAAR4
#define INV_HAAR4(a,b,c,d, o0,o1,o2,o3, t0,t1,t2,t3,t4) do { \
    (void)(a); (void)(b); (void)(c); (void)(d); \
    (void)(t0); (void)(t1); (void)(t2); (void)(t3); (void)(t4); \
    /* keep side-effects minimal; arguments are evaluated before this macro */ \
} while (0)
#endif

// Vulnerable function (kept verbatim around the sink)
void ff_ivi_col_haar4(const int32_t *in, int16_t *out, ptrdiff_t pitch,
                      const uint8_t *flags)
{
    int     i;
    int     t0, t1, t2, t3, t4;

    /* apply the InvHaar8 to all columns */
#define COMPENSATE(x) (x)
    for (i = 0; i < 4; i++) {
        if (flags[i]) {
            INV_HAAR4(in[0], in[4], in[8], in[12],
                      out[0 * pitch], out[1 * pitch],
                      out[2 * pitch], out[3 * pitch],
                      t0, t1, t2, t3, t4);
            klee_assert(0 && "SAILOR_SINK_REACHED");
        } else
            out[0 * pitch] = out[1 * pitch] =
            out[2 * pitch] = out[3 * pitch] = 0;

        in++;
        out++;
    }
#undef  COMPENSATE
}

// Entry function must be a pure pass-through wrapper with no guards
int entry_func(const int32_t *in, int16_t *out, ptrdiff_t pitch,
               const uint8_t *flags) {
    ff_ivi_col_haar4(in, out, pitch, flags);
    return 0;
}
