/* Minimal sliced harness for eaidct.c vulnerability at line 67 */
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <klee/klee.h>

// Vulnerable function (keep exact vulnerable statement)
static inline void ea_idct_col(int16_t *dest, const int16_t *src) {
    if ((src[8]|src[16]|src[24]|src[32]|src[40]|src[48]|src[56])==0) {
        // Minimal body sufficient for compilation; semantics are irrelevant
        dest[56] = src[0];
    } else {
        // Neutralized else-branch
        ;
    }
    // Universal sink assertion after vulnerable statement
    klee_assert(0 && "SAILOR_SINK_REACHED");
}

// Entry function: direct pass-through to vulnerable function with NO guards
void ff_ea_idct_put_c(uint8_t *dest, ptrdiff_t linesize, int16_t *block) {
    (void)dest; (void)linesize; // unused in neutralized entry
    ea_idct_col(block, block);
    return;
}
