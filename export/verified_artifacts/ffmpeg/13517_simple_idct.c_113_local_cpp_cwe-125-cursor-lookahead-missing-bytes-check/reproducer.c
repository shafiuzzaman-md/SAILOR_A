// Combined reproducer for 13517_simple_idct.c_113_local_cpp_cwe-125-cursor-lookahead-missing-bytes-check
// Original harness: driver.c + smart_stubs.c + sliced source

// === smart_stubs.c ===
/* Smart stubs — auto-generated from path + vulnerability analysis */
/* Symbolic stubs model the environment: KLEE explores return values */
/* that both REACH the sink AND TRIGGER the vulnerability */
#include <stdlib.h>
#include <string.h>
#include <klee/klee.h>

/* PROACTIVE: assertion (auto-detected external) */
int assertion() { return 0; }

/* PROACTIVE: function (auto-detected external) */
int function() { return 0; }

// === driver.c ===
// NO_HARNESS_TYPES
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <klee/klee.h>

int entry_func(uint8_t *dest, ptrdiff_t line_size, int16_t *block);

int main() {
    // Allocate a small destination buffer
    uint8_t *dest = (uint8_t *)malloc(64);
    klee_make_symbolic(dest, 64, "dest_buf");

    // Use a concrete line_size
    ptrdiff_t line_size = 16;

    // Allocate an intentionally too-small block: only 8 int16_t elements
    // BF(0) will access ptr[8], which is OOB for this allocation
    size_t block_elems = 8; // deliberately < 9 to trigger OOB at ptr[8]
    int16_t *block = (int16_t *)malloc(block_elems * sizeof(int16_t));
    klee_make_symbolic(block, block_elems * sizeof(int16_t), "block_buf");

    // Call entry (pure pass-through to vulnerable function)
    entry_func(dest, line_size, block);
    return 0;
}
