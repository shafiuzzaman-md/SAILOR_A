// Combined reproducer for 7874_ivi_dsp.c_459_local_cpp_cwe-125-cursor-lookahead-missing-bytes-check
// Original harness: driver.c + smart_stubs.c + sliced source

// === smart_stubs.c ===
/* Smart stubs — auto-generated from path + vulnerability analysis */
/* Symbolic stubs model the environment: KLEE explores return values */
/* that both REACH the sink AND TRIGGER the vulnerability */
#include <stdlib.h>
#include <string.h>
#include <klee/klee.h>

/* PROACTIVE: COMPENSATE (auto-detected external) */
int COMPENSATE() { return 0; }

/* PROACTIVE: function (auto-detected external) */
int function() { return 0; }

// === driver.c ===
// NO_HARNESS_TYPES
#include <klee/klee.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>

int entry_func(const int32_t *in, int16_t *out, ptrdiff_t pitch, const uint8_t *flags);

int main() {
    // Input buffer: 16 int32_t elements
    int32_t *inbuf = (int32_t *)calloc(16, sizeof(int32_t));
    if (!inbuf) return 0;
    klee_make_symbolic(inbuf, 16 * sizeof(int32_t), "inbuf_bytes");

    // Symbolic starting index so that in[12] may go OOB
    int in_idx;
    klee_make_symbolic(&in_idx, sizeof(in_idx), "in_idx");
    klee_assume(in_idx >= 5);
    klee_assume(in_idx < 16);
    const int32_t *in = inbuf + in_idx;

    // Output buffer and pitch
    int16_t *outbuf = (int16_t *)calloc(64, sizeof(int16_t));
    if (!outbuf) return 0;
    ptrdiff_t pitch = 1;

    // Flags: ensure first iteration executes the vulnerable path
    uint8_t flags[4];
    klee_make_symbolic(flags, sizeof(flags), "flags");
    klee_assume(flags[0] != 0);

    // Call entry
    entry_func(in, outbuf, pitch, flags);
    return 0;
}
