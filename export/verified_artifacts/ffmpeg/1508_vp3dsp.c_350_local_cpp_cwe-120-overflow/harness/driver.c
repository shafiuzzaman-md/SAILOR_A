#include "harness_types.h"
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <klee/klee.h>

// entry_func prototype from harness
int entry_func(uint8_t *dest, ptrdiff_t stride, int16_t *block);

int main() {
    // Allocate a small destination buffer (not used by stubbed idct10)
    uint8_t *dest = (uint8_t *)malloc(64);
    klee_make_symbolic(dest, 64, "dest_buf");

    // Stride can be symbolic; it's unused in our neutralized path
    ptrdiff_t stride;
    klee_make_symbolic(&stride, sizeof(stride), "stride");

    // INTENTIONAL: allocate too-small block to trigger overflow in memset(block, 0, sizeof(*block)*64)
    // Needed size would be 64 * sizeof(int16_t) = 128 bytes; we give only 32 elements = 64 bytes
    int16_t *block = (int16_t *)malloc(32 * sizeof(int16_t));
    klee_make_symbolic(block, 32 * sizeof(int16_t), "block_buf");

    // Direct call to entry (which directly calls the vulnerable function)
    entry_func(dest, stride, block);
    return 0;
}
