// harness/driver.c
#include "harness_types.h"
// klee removed for replay
#include <stdlib.h>
#include <string.h>

int main() {
    // Allocate context and buffers
    fz_context *ctx = (fz_context *)calloc(1, sizeof(fz_context));
    fz_buffer *buf = (fz_buffer *)calloc(1, sizeof(fz_buffer));
    fz_buffer *extra = (fz_buffer *)calloc(1, sizeof(fz_buffer));

    // Set up OOB condition: len > cap so (cap - len) underflows (size_t)
    buf->cap = 16;
    buf->len = 32; // greater than cap -> underflow ensures no reallocation path taken
    buf->data = (unsigned char *)malloc(buf->cap);
    if (!buf->data) return 0;
    { static const unsigned char buf_data_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(buf->data, buf_data_data, (buf->cap < sizeof(buf_data_data)) ? buf->cap : sizeof(buf_data_data)); };

    // Extra buffer with concrete, small length
    extra->len = 8;
    extra->data = (unsigned char *)malloc(extra->len);
    if (!extra->data) return 0;
    { static const unsigned char extra_data_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(extra->data, extra_data_data, (extra->len < sizeof(extra_data_data)) ? extra->len : sizeof(extra_data_data)); };

    // Call entry/vulnerable function directly
    fz_append_buffer(ctx, buf, extra);

    return 0;
}
