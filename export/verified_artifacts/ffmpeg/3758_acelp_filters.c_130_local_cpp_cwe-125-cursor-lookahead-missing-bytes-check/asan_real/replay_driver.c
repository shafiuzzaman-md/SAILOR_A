// Combined reproducer for 3758_acelp_filters.c_130_local_cpp_cwe-125-cursor-lookahead-missing-bytes-check
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
// NO_HARNESS_TYPES
#include <klee/klee.h>
#include <stdlib.h>
#include <string.h>

// Prototype from harness
int entry_func(float *out, const float *in,
               const float zero_coeffs[2],
               const float pole_coeffs[2],
               float gain, float mem[2], int n);

int main() {
    // Bound for iterations
    int n;
    klee_make_symbolic(&n, sizeof(n), "n");
    klee_assume(n >= 1);
    klee_assume(n <= 8);

    // Allocate concrete buffers (sizes must be concrete numbers)
    float *out = (float *)malloc(sizeof(float) * 8);
    float *in  = (float *)malloc(sizeof(float) * 8);
    float *zero_coeffs = (float *)malloc(sizeof(float) * 2);
    float *mem = (float *)malloc(sizeof(float) * 2);

    // Intentionally under-allocate pole_coeffs to 1 float to trigger OOB read at pole_coeffs[1]
    float *pole_coeffs = (float *)malloc(sizeof(float) * 1);

    // Make contents symbolic
    klee_make_symbolic(out, sizeof(float) * 8, "out_buf");
    klee_make_symbolic(in,  sizeof(float) * 8, "in_buf");
    klee_make_symbolic(zero_coeffs, sizeof(float) * 2, "zero_coeffs");
    klee_make_symbolic(mem, sizeof(float) * 2, "mem");
    klee_make_symbolic(pole_coeffs, sizeof(float) * 1, "pole_coeffs");

    float gain;
    klee_make_symbolic(&gain, sizeof(gain), "gain");

    // Call entry (direct pass-through to vulnerable function)
    entry_func(out, in, zero_coeffs, pole_coeffs, gain, mem, n);

    return 0;
}
