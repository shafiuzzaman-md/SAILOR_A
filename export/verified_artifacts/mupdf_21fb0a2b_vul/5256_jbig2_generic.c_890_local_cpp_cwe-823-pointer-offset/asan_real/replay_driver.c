#include "harness_types.h"
// klee removed for replay
#include <stdlib.h>
#include <string.h>

int main() {
    // Allocate required structs
    Jbig2Ctx *ctx = (Jbig2Ctx *)calloc(1, sizeof(Jbig2Ctx));
    Jbig2Segment *segment = (Jbig2Segment *)calloc(1, sizeof(Jbig2Segment));
    Jbig2GenericRegionParams *params = (Jbig2GenericRegionParams *)calloc(1, sizeof(Jbig2GenericRegionParams));
    Jbig2ArithState *as = (Jbig2ArithState *)calloc(1, sizeof(Jbig2ArithState));
    Jbig2ArithCx *GB_stats = (Jbig2ArithCx *)calloc(1, sizeof(Jbig2ArithCx));
    Jbig2Image *image = (Jbig2Image *)calloc(1, sizeof(Jbig2Image));

    // Concrete sizes per guidance (no symbolic malloc sizes)
    const uint32_t WIDTH = 8;   // arbitrary
    const uint32_t HEIGHT = 1;  // single row so dest points just past buffer
    const uint32_t STRIDE = 8;  // bytes per row

    image->width = WIDTH;
    image->height = HEIGHT;
    image->stride = STRIDE;

    // Allocate exactly height * stride bytes
    size_t buf_size = (size_t)HEIGHT * (size_t)STRIDE;
    image->data = (uint8_t *)malloc(buf_size);
    if (!image->data) return 0;

    // Make source row bytes symbolic (read side of memcpy)
    { static const unsigned char img_data_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(image->data, img_data_data, (buf_size < sizeof(img_data_data)) ? buf_size : sizeof(img_data_data)); };

    // Call entry function (neutralized to directly reach copy_prev_row)
    jbig2_decode_generic_region(ctx, segment, params, as, image, GB_stats);

    return 0;
}
