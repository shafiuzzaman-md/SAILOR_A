#include "harness_types.h"
// klee removed for replay
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#ifndef BUF_SIZE
#define BUF_SIZE 64
#endif

int main() {
    // Allocate context and bitmap structures
    fz_context *ctx = (fz_context *)calloc(1, sizeof(fz_context));
    fz_bitmap *bit = (fz_bitmap *)calloc(1, sizeof(fz_bitmap));

    // Allocate a small samples buffer
    unsigned char *buf = (unsigned char *)malloc(BUF_SIZE);
    if (!ctx || !bit || !buf) return 0;

    // Make buffer contents symbolic (not strictly required, but standard practice)
    { static const unsigned char samples_buf_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(buf, samples_buf_data, (BUF_SIZE < sizeof(samples_buf_data)) ? BUF_SIZE : sizeof(samples_buf_data)); };

    bit->samples = buf;

    // Set stride and height so that (size_t)stride * h > BUF_SIZE to trigger overflow
    // Keep values modest to avoid huge memset operations
    bit->stride = 80;  // 80 * 2 = 160 > 64
    bit->h = 2;

    // Call entry/vulnerable function directly
    fz_clear_bitmap(ctx, bit);

    return 0;
}
