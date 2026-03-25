// Combined reproducer for 2934_murmur3.c_129_local_cpp_cwe-120-overflow
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
#include "harness_types.h"
#include <klee/klee.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

int main() {
    // Allocate context struct with concrete size
    AVMurMur3 *c = (AVMurMur3 *)calloc(1, sizeof(AVMurMur3));

    // Small concrete source buffer
    const size_t SRC_SIZE = 4;  // small so (len & 15) > SRC_SIZE can overflow read
    uint8_t *src = (uint8_t *)malloc(SRC_SIZE);
    klee_make_symbolic(src, SRC_SIZE, "src_bytes");

    // Symbolic length. We will constrain the tail (len & 15) to exceed SRC_SIZE
    size_t len;
    klee_make_symbolic(&len, sizeof(len), "len");

    // Force the tail copy to execute and exceed source size to trigger OOB read in memcpy
    size_t tail = (len & 15);
    klee_assume(tail > SRC_SIZE);   // ensures memcpy reads past src buffer
    klee_assume(tail <= 15);        // redundant but explicit bound

    // Direct call into the entry (pass-through to vulnerable function)
    entry_func(c, src, len);
    return 0;
}
