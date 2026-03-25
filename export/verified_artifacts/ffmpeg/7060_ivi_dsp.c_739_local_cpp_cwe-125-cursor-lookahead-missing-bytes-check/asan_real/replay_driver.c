// Combined reproducer for 7060_ivi_dsp.c_739_local_cpp_cwe-125-cursor-lookahead-missing-bytes-check
// Original harness: driver.c + smart_stubs.c + sliced source

// === smart_stubs.c ===
/* Smart stubs — auto-generated from path + vulnerability analysis */
/* Symbolic stubs model the environment: KLEE explores return values */
/* that both REACH the sink AND TRIGGER the vulnerability */
#include <stdlib.h>
#include <string.h>
#include <klee/klee.h>

/* PROACTIVE: arguments (auto-detected external) */
int arguments() { return 0; }

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

// entry_func is defined in harness/ivi_dsp.c
int entry_func(const int32_t *in, int16_t *out, ptrdiff_t pitch, const uint8_t *flags);

int main() {
    // Allocate a deliberately too-small input buffer to trigger OOB on in[8]/in[12]
    const size_t in_len = 8; // fewer than 13 elements used by the call site
    int32_t *in = (int32_t *)malloc(in_len * sizeof(int32_t));
    klee_make_symbolic(in, in_len * sizeof(int32_t), "in_buf");

    // Output buffer must cover indices: 0, pitch, row2, row2+pitch, with row2 = pitch<<1
    ptrdiff_t pitch = 1;
    const size_t out_len = 4; // for pitch=1, need at least 4 elements
    int16_t *out = (int16_t *)malloc(out_len * sizeof(int16_t));
    klee_make_symbolic(out, out_len * sizeof(int16_t), "out_buf");

    // Flags: ensure flags[0] is true to take the vulnerable path
    uint8_t *flags = (uint8_t *)malloc(4);
    klee_make_symbolic(flags, 4, "flags");
    klee_assume(flags[0] != 0);

    // Direct call into the entry function
    entry_func(in, out, pitch, flags);
    return 0;
}
