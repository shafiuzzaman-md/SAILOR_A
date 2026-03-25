#include <string.h>
#include "harness_types.h"
// klee removed for replay

int main() {
    // We don't need concrete struct layouts because our harness doesn't dereference them.
    fz_context *ctx = 0;
    const fz_pixmap *src = 0;
    fz_pixmap *dst = 0;
    fz_colorspace *is = 0;
    fz_color_params params; // unused in harness

    int copy_spots;
    { static const unsigned char copy_spots_data[] = {0x00, 0x00, 0x00, 0x00}; memcpy(&copy_spots, copy_spots_data, (sizeof(copy_spots) < sizeof(copy_spots_data)) ? sizeof(copy_spots) : sizeof(copy_spots_data)); };

    fz_convert_slow_pixmap_samples(ctx, src, dst, is, params, copy_spots);
    return 0;
}
