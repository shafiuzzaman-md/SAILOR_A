#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <klee/klee.h>

// Minimal neutralized helper to satisfy call before the vulnerable memset
static void idct10(uint8_t *dest, ptrdiff_t stride, int16_t *block, int e) {
    (void)dest; (void)stride; (void)block; (void)e;
}

// Vulnerable function (from vp3dsp.c around line 350)
void ff_vp3dsp_idct10_add(uint8_t *dest, ptrdiff_t stride, int16_t *block)
{
    // keep signature and minimal structure
    idct10(dest, stride, block, 2);
    // VULNERABLE STATEMENT — must be verbatim
    memset(block, 0, sizeof(*block) * 64);
    // Universal sink assertion (fires if no crash)
    klee_assert(0 && "SAILOR_SINK_REACHED");
}

// ENTRY: direct pass-through with no guards
int entry_func(uint8_t *dest, ptrdiff_t stride, int16_t *block) {
    ff_vp3dsp_idct10_add(dest, stride, block);
    return 0;
}
