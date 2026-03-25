// Combined reproducer for 6964_ivi_dsp.c_767_local_cpp_cwe-125-cursor-lookahead-missing-bytes-check
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

/* PROACTIVE: statement (auto-detected external) */
int statement() { return 0; }

// === driver.c ===
// NO_HARNESS_TYPES
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <klee/klee.h>

// Forward declaration of entry function from harness
int entry_func(const int32_t *in, int16_t *out, ptrdiff_t pitch, int blk_size);

int main() {
    // Allocate a too-small concrete input buffer (1 byte) to force OOB read of in[0]
    void *in_raw = malloc(1);
    klee_make_symbolic(in_raw, 1, "in_raw");

    // Output buffer with at least one element
    int16_t outbuf[8];
    klee_make_symbolic(outbuf, sizeof(outbuf), "outbuf");

    // Call entry; pitch and blk_size are irrelevant in the sliced function
    entry_func((const int32_t*)in_raw, outbuf, 0, 0);
    return 0;
}
