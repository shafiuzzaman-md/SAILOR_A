// Combined reproducer for 4403_eaidct.c_67_local_cpp_cwe-125-cursor-lookahead-missing-bytes-check
// Original harness: driver.c + smart_stubs.c + sliced source

// === smart_stubs.c ===
/* Smart stubs — auto-generated from path + vulnerability analysis */
/* Symbolic stubs model the environment: KLEE explores return values */
/* that both REACH the sink AND TRIGGER the vulnerability */
#include <stdlib.h>
#include <string.h>
#include <klee/klee.h>

/* PROACTIVE: function (auto-detected external) */
int function() { return 0; }

// === driver.c ===
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
