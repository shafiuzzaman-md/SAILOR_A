// Combined reproducer for 7060_ivi_dsp.c_739_local_cpp_cwe-125-cursor-lookahead-missing-bytes-check
// Original harness: driver.c + smart_stubs.c + sliced source

// === smart_stubs.c ===
/* Smart stubs — auto-generated from path + vulnerability analysis */
/* Symbolic stubs model the environment: KLEE explores return values */
/* that both REACH the sink AND TRIGGER the vulnerability */
#include <stdlib.h>
#include <string.h>
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
// entry_func is defined in harness/ivi_dsp.c
int entry_func(const int32_t *in, int16_t *out, ptrdiff_t pitch, const uint8_t *flags);

int LLVMFuzzerTestOneInput(const uint8_t *fuzz_data, size_t fuzz_size) {
    if (fuzz_size < 64) return 0;
    // Allocate a deliberately too-small input buffer to trigger OOB on in[8]/in[12]
    const size_t in_len = 8; // fewer than 13 elements used by the call site
    int32_t *in = (int32_t *)malloc(in_len * sizeof(int32_t));
    memcpy(in, fuzz_data + (0), in_len * sizeof(int32_t));

    // Output buffer must cover indices: 0, pitch, row2, row2+pitch, with row2 = pitch<<1
    ptrdiff_t pitch = 1;
    const size_t out_len = 4; // for pitch=1, need at least 4 elements
    int16_t *out = (int16_t *)malloc(out_len * sizeof(int16_t));
    memcpy(out, fuzz_data + (in_len * sizeof(int32_t)), out_len * sizeof(int16_t));

    // Flags: ensure flags[0] is true to take the vulnerable path
    uint8_t *flags = (uint8_t *)malloc(4);
    memcpy(flags, fuzz_data + (in_len * sizeof(int32_t) + out_len * sizeof(int16_t)), 4);
    

    // Direct call into the entry function
    entry_func(in, out, pitch, flags);
    return 0;
}
