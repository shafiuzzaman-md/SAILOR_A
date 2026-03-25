#include "harness_types.h"
#include <klee/klee.h>
#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>

int main() {
    // Allocate a minimal dest buffer (unused in neutralized entry)
    uint8_t *dest = (uint8_t *)malloc(16);
    klee_make_symbolic(dest, 16, "dest_buf");

    // linesize can be any concrete value; not used in our neutralized entry
    ptrdiff_t linesize = 8;

    // Allocate a SMALL int16_t block so that accessing src[8] is OOB
    // Size 8 means valid indices are 0..7; src[8] is out-of-bounds
    int16_t *block = (int16_t *)malloc(8 * sizeof(int16_t));
    klee_make_symbolic(block, 8 * sizeof(int16_t), "block_buf");

    // Direct call to entry which immediately calls ea_idct_col(block, block)
    ff_ea_idct_put_c(dest, linesize, block);
    return 0;
}
