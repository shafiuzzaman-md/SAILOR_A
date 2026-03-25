#include "harness_types.h"
// klee removed for replay
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

int main() {
    // Allocate context and image with concrete sizes
    Jbig2Ctx *ctx = (Jbig2Ctx *)calloc(1, sizeof(Jbig2Ctx));
    if (!ctx) return 0;
    ctx->allocator = (Jbig2Allocator *)calloc(1, sizeof(Jbig2Allocator));

    Jbig2Image *image = (Jbig2Image *)calloc(1, sizeof(Jbig2Image));
    if (!image) return 0;

    // Initialize fields to satisfy guards and reach the vulnerable memset
    // Choose small, concrete values to allow allocation to succeed
    image->width = 16;              // any value
    image->height = 2;              // old height
    image->stride = 8;              // bytes per row, non-zero

    // Satisfy overflow guard: height <= INT32_MAX / stride (holds for our small numbers)
    // Allocate initial data buffer of size old_height * stride
    size_t old_size = (size_t)image->height * image->stride; // 2 * 8 = 16
    image->data = (uint8_t *)malloc(old_size);
    if (!image->data) return 0;
    { static const unsigned char image_data_initial_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(image->data, image_data_initial_data, (old_size < sizeof(image_data_initial_data)) ? old_size : sizeof(image_data_initial_data)); };

    // New dimensions to trigger the memset in the kept path
    uint32_t new_width = image->width;  // original branch used width == image->width
    uint32_t new_height = 4;            // greater than old height to enter memset block

    int value;                          // fill selector
    { static const unsigned char fill_value_data[] = {0x00, 0x00, 0x00, 0x00}; memcpy(&value, fill_value_data, (sizeof(value) < sizeof(fill_value_data)) ? sizeof(value) : sizeof(fill_value_data)); };

    // Call entry/vulnerable function directly
    jbig2_image_resize(ctx, image, new_width, new_height, value);

    return 0;
}
