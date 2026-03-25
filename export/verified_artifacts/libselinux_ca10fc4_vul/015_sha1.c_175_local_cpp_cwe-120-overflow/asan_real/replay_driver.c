#include <string.h>
#include "harness_types.h"
// klee removed for replay
#include <stdint.h>
#include <stdlib.h>

int Sha1Update(SHA1Context *ctx, const uint8_t *data, uint32_t len);

int main() {
    SHA1Context *ctx = (SHA1Context *)calloc(1, sizeof(SHA1Context));

    // Force j = 0 so i = 64 in the target branch
    ctx->Count[0] = 0;
    ctx->Count[1] = 0;

    // Intentionally SMALL source buffer to trigger memcpy over-read
    const unsigned DATA_CAP = 32; // smaller than i (64)
    uint8_t *buf = (uint8_t *)malloc(DATA_CAP);
    { static const unsigned char sha1_buf_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(buf, sha1_buf_data, (DATA_CAP < sizeof(sha1_buf_data)) ? DATA_CAP : sizeof(sha1_buf_data)); };

    // Symbolic length: ensure (j + BufferSize) > 63 and len > DATA_CAP
    uint32_t len;
    { static const unsigned char sha1_len_data[] = {0xc0, 0x00, 0x00, 0x00}; memcpy(&len, sha1_len_data, (sizeof(len) < sizeof(sha1_len_data)) ? sizeof(len) : sizeof(sha1_len_data)); };
    /* klee_assume removed */
    /* klee_assume removed */

    Sha1Update(ctx, buf, len);
    return 0;
}
