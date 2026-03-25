// Combined reproducer for 1508_vp3dsp.c_350_local_cpp_cwe-120-overflow
// Original harness: driver.c + smart_stubs.c + sliced source

// === smart_stubs.c ===
/* Smart stubs — auto-generated from path + vulnerability analysis */
/* Symbolic stubs model the environment: KLEE explores return values */
/* that both REACH the sink AND TRIGGER the vulnerability */
#include <stdlib.h>
#include <string.h>
/* PROACTIVE: assertion (auto-detected external) */
int assertion() { return 0; }

/* PROACTIVE: function (auto-detected external) */
int function() { return 0; }

// === driver.c ===
#include "harness_types.h"
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
// entry_func prototype from harness
int entry_func(uint8_t *dest, ptrdiff_t stride, int16_t *block);

int LLVMFuzzerTestOneInput(const uint8_t *fuzz_data, size_t fuzz_size) {
    if (fuzz_size < 64) return 0;
    // Allocate a small destination buffer (not used by stubbed idct10)
    uint8_t *dest = (uint8_t *)malloc(64);
    memcpy(dest, fuzz_data + (0), 64);

    // Stride can be symbolic; it's unused in our neutralized path
    ptrdiff_t stride;
    memcpy(&stride, fuzz_data + (64), sizeof(stride));

    // INTENTIONAL: allocate too-small block to trigger overflow in memset(block, 0, sizeof(*block)*64)
    // Needed size would be 64 * sizeof(int16_t) = 128 bytes; we give only 32 elements = 64 bytes
    int16_t *block = (int16_t *)malloc(32 * sizeof(int16_t));
    memcpy(block, fuzz_data + (64 + sizeof(stride)), 32 * sizeof(int16_t));

    // Direct call to entry (which directly calls the vulnerable function)
    entry_func(dest, stride, block);
    return 0;
}
