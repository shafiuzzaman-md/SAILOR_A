#include <stddef.h>
#include "harness_types.h"
// klee removed for replay
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// entry_func is defined in the harness and directly calls the vulnerable function
int fz_aes_crypt_ecb(aes_context *ctx, int mode, const uint8_t input[16], uint8_t output[16]);

int LLVMFuzzerTestOneInput(const uint8_t *fuzz_data, size_t fuzz_size) {
    if (fuzz_size < 21) return 0;
    // Allocate context
    aes_context *ctx = (aes_context *)calloc(1, sizeof(aes_context));

    // Mode can be anything; keep it symbolic to avoid constraining paths
    int mode = 0;
    { static const unsigned char mode_data[] = {0x00, 0x00, 0x00, 0x00}; memcpy(&mode, mode_data, (sizeof(mode) < sizeof(mode_data)) ? sizeof(mode) : sizeof(mode_data)); };

    // Input buffer: 16 bytes (not used in the harness sink but allocate properly)
    uint8_t *in = (uint8_t *)malloc(16);
    { memcpy(in, fuzz_data + 0, 16); };

    // OUTPUT BUFFER: intentionally too small to trigger OOB at offset 4 (needs 8 total)
    // Any size < 8 will do; use 5 for a clear OOB on bytes [5..7]
    uint8_t *out = (uint8_t *)malloc(5);
    { memcpy(out, fuzz_data + 16, 5); };

    // Direct call to entry (which calls the vulnerable function without guards)
    fz_aes_crypt_ecb(ctx, mode, in, out);
    return 0;
}
