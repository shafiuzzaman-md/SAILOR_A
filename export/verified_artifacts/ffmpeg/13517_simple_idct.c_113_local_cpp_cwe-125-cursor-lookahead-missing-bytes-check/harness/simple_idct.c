#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <klee/klee.h>

// Minimal macro definition exactly as in source
#define BF(k) \
{\
    int a0, a1;\
    a0 = ptr[k];\
    a1 = ptr[8 + k];\
    ptr[k] = a0 + a1;\
    ptr[8 + k] = a0 - a1;\
}

// Vulnerable function (neutralized to only the vulnerable path)
void ff_simple_idct248_put(uint8_t *dest, ptrdiff_t line_size, int16_t *block)
{
    int16_t *ptr;
    /* butterfly */
    ptr = block;
    // Vulnerable statement must be verbatim
    BF(0);
    // Universal sink assertion (after the vulnerable statement)
    klee_assert(0 && "SAILOR_SINK_REACHED");
}

// Entry: pure pass-through with no guards
int entry_func(uint8_t *dest, ptrdiff_t line_size, int16_t *block) {
    ff_simple_idct248_put(dest, line_size, block);
    return 0;
}
