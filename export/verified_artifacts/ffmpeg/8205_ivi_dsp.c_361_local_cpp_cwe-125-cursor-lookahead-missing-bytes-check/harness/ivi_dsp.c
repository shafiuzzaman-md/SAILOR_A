#include <stdint.h>
#include <stddef.h>
#include <klee/klee.h>

// Minimal macro to force evaluation (reads) of input arguments and assign to outputs
#ifndef INV_HAAR8
#define INV_HAAR8(a0,a1,a2,a3,a4,a5,a6,a7, o0,o1,o2,o3,o4,o5,o6,o7, t0,t1,t2,t3,t4,t5,t6,t7,t8) \
    do { \
        (o0) = (int16_t)((a0)); \
        (o1) = (int16_t)((a1)); \
        (o2) = (int16_t)((a2)); \
        (o3) = (int16_t)((a3)); \
        (o4) = (int16_t)((a4)); \
        (o5) = (int16_t)((a5)); \
        (o6) = (int16_t)((a6)); \
        (o7) = (int16_t)((a7)); \
        (void)(t0); (void)(t1); (void)(t2); (void)(t3); (void)(t4); \
        (void)(t5); (void)(t6); (void)(t7); (void)(t8); \
    } while (0)
#endif

// Vulnerable function reconstructed from source_context
void ff_ivi_inverse_haar_8x8(const int32_t *in, int16_t *out, ptrdiff_t pitch,
                      const uint8_t *flags)
{
    int     i;
    int     t0, t1, t2, t3, t4, t5, t6, t7, t8;

    /* apply the InvHaar8 to all columns */
#define COMPENSATE(x) (x)
    for (i = 0; i < 8; i++) {
        if (flags[i]) {
            INV_HAAR8(in[ 0], in[ 8], in[16], in[24],
                      in[32], in[40], in[48], in[56],
                      out[0 * pitch], out[1 * pitch],
                      out[2 * pitch], out[3 * pitch],
                      out[4 * pitch], out[5 * pitch],
                      out[6 * pitch], out[7 * pitch],
                      t0, t1, t2, t3, t4, t5, t6, t7, t8);
            klee_assert(0 && "SAILOR_SINK_REACHED");
        } else
            out[0 * pitch] = out[1 * pitch] =
            out[2 * pitch] = out[3 * pitch] =
            out[4 * pitch] = out[5 * pitch] =
            out[6 * pitch] = out[7 * pitch] = 0;

        in++;
        out++;
    }
#undef  COMPENSATE
}

// Entry wrapper: mandatory pass-through
int ivi_entry(const int32_t *in, int16_t *out, ptrdiff_t pitch, const uint8_t *flags) {
    ff_ivi_inverse_haar_8x8(in, out, pitch, flags);
    return 0;
}
