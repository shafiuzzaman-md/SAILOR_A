#include "harness_types.h"
// klee removed for replay
#include <stdlib.h>
#include <string.h>

int main() {
    // Allocate trivial context and colorspaces
    fz_context *ctx = (fz_context *)calloc(1, sizeof(fz_context));
    fz_colorspace *src_cs = (fz_colorspace *)calloc(1, sizeof(fz_colorspace));
    fz_colorspace *is = (fz_colorspace *)calloc(1, sizeof(fz_colorspace));
    fz_color_params params; memset(&params, 0, sizeof(params));

    // One pixel images
    const int w = 1, h = 1;

    // Source pixmap: minimal but valid pointers
    fz_pixmap *src = (fz_pixmap *)calloc(1, sizeof(fz_pixmap));
    src->w = w; src->h = h;
    src->alpha = 0;      // no alpha in src
    src->s = 0;          // no spots in src
    src->n = 3;          // 3 components (e.g., RGB)
    src->stride = src->n; // tightly packed single pixel
    src->colorspace = src_cs;
    src->samples = (unsigned char *)malloc(8); // enough so s and sold are non-NULL
    { static const unsigned char src_samples_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(src->samples, src_samples_data, (8 < sizeof(src_samples_data)) ? 8 : sizeof(src_samples_data)); };

    // Destination pixmap: craft for overflow in memset(d + dst_c, 0, dst_s)
    // Choose dst_n = dst_s (da=0) so dst_c = dst_n - dst_s - da = 0.
    // This skips the preceding memcpy and ensures memset starts at d (offset 0).
    // Allocate ONLY 2 bytes for d, but set dst_s = dst_n = 3 so memset overflows.
    fz_pixmap *dst = (fz_pixmap *)calloc(1, sizeof(fz_pixmap));
    dst->w = w; dst->h = h;
    dst->alpha = 0;  // da = 0
    dst->s = 3;      // dst_s
    dst->n = 3;      // dst_n = dst_s => dst_c = 0
    dst->stride = dst->n; // tightly packed single pixel
    dst->colorspace = src_cs; // any non-NULL
    dst->samples = (unsigned char *)malloc(2); // intentionally too small
    { static const unsigned char dst_samples_data[] = {0x00, 0x00}; memcpy(dst->samples, dst_samples_data, (2 < sizeof(dst_samples_data)) ? 2 : sizeof(dst_samples_data)); };

    // Call entry (neutralized to directly call memoize_spots)
    fz_convert_slow_pixmap_samples(ctx, src, dst, is, params, 0);

    return 0;
}
