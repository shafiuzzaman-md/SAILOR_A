// Combined reproducer for 7137_ivi_dsp.c_716_local_cpp_cwe-125-cursor-lookahead-missing-bytes-check
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

/* PROACTIVE: decls (auto-detected external) */
int decls() { return 0; }

// === driver.c ===
#include "harness_types.h"
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <klee/klee.h>

int harness_entry(const int32_t *in, int16_t *out, ptrdiff_t pitch, const uint8_t *flags);

int main() {
    // Allocate undersized input buffer: only 3 int32_t elements
    int32_t *in = (int32_t *)malloc(3 * sizeof(int32_t));
    if (!in) return 0;
    klee_make_symbolic(in, 3 * sizeof(int32_t), "in_buf");

    // Output buffer and parameters (not used before the sink in our slice)
    int16_t *out = (int16_t *)malloc(4 * sizeof(int16_t));
    if (!out) return 0;
    klee_make_symbolic(out, 4 * sizeof(int16_t), "out_buf");

    ptrdiff_t pitch = 4;  // one row of 4 elements
    uint8_t *flags = (uint8_t *)malloc(4);
    if (!flags) return 0;
    klee_make_symbolic(flags, 4, "flags_buf");

    // Direct call into the harness entry which calls the vulnerable function
    harness_entry(in, out, pitch, flags);
    return 0;
}
