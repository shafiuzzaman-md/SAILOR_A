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

int main() {
    aes_context ctx; memset(&ctx, 0, sizeof(ctx));

    // Prepare buffers: IV is intentionally undersized to trigger overflow at memcpy(iv, temp, 16)
    uint8_t *iv = (uint8_t*)malloc(8);      // smaller than 16 → overflow sink
    uint8_t *input = (uint8_t*)malloc(16);  // at least one block
    uint8_t *output = (uint8_t*)malloc(16); // one block output

    if (iv) { static const unsigned char iv_bytes_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(iv, iv_bytes_data, (8 < sizeof(iv_bytes_data)) ? 8 : sizeof(iv_bytes_data)); };
    if (input) { static const unsigned char input_bytes_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(input, input_bytes_data, (16 < sizeof(input_bytes_data)) ? 16 : sizeof(input_bytes_data)); };
    if (output) { static const unsigned char output_bytes_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(output, output_bytes_data, (16 < sizeof(output_bytes_data)) ? 16 : sizeof(output_bytes_data)); };

    int mode = FZ_AES_DECRYPT;
    size_t length = 16; // process exactly one block

    fz_aes_crypt_cbc(&ctx, mode, length, iv, input, output);
    return 0;
}
