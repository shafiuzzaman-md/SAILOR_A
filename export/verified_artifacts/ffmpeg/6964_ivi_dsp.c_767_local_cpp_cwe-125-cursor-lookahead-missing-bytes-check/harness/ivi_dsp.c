#include <stddef.h>
#include <stdint.h>
#include <klee/klee.h>

// Neutralized vulnerable function from ivi_dsp.c around line 767
void ff_ivi_put_dc_pixel_8x8(const int32_t *in, int16_t *out, ptrdiff_t pitch,
                             int blk_size)
{
    // Vulnerable statement (must be verbatim):
    out[0] = in[0];
    // Universal sink assertion — fires only if the above didn't crash
    klee_assert(0 && "SAILOR_SINK_REACHED");
}

// Mandatory pass-through entry function (no guards)
int entry_func(const int32_t *in, int16_t *out, ptrdiff_t pitch, int blk_size) {
    ff_ivi_put_dc_pixel_8x8(in, out, pitch, blk_size);
    return 0;
}
