#include <stddef.h>
// NO_HARNESS_TYPES
// klee removed for replay
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifndef FZ_AES_DECRYPT
#define FZ_AES_DECRYPT 0
#endif

typedef struct aes_context { int nr; } aes_context;

int fz_aes_crypt_cbc(aes_context *ctx, int mode, size_t length,
                  uint8_t *iv, const uint8_t *input, uint8_t *output);

int LLVMFuzzerTestOneInput(const uint8_t *fuzz_data, size_t fuzz_size) {
    if (fuzz_size < 40) return 0;
    aes_context ctx; memset(&ctx, 0, sizeof(ctx));

    // Prepare buffers: IV is intentionally undersized to trigger overflow at memcpy(iv, temp, 16)
    uint8_t *iv = (uint8_t*)malloc(8);      // smaller than 16 → overflow sink
    uint8_t *input = (uint8_t*)malloc(16);  // at least one block
    uint8_t *output = (uint8_t*)malloc(16); // one block output

    if (iv) { memcpy(iv, fuzz_data + 0, 8); };
    if (input) { memcpy(input, fuzz_data + 8, 16); };
    if (output) { memcpy(output, fuzz_data + 24, 16); };

    int mode = FZ_AES_DECRYPT;
    size_t length = 16; // process exactly one block

    fz_aes_crypt_cbc(&ctx, mode, length, iv, input, output);
    return 0;
}
