#include "harness_types.h"
#include <stdlib.h>
#include <string.h>
// klee removed for replay

int main() {
    // Concrete allocations per guidance
    fz_context *ctx = (fz_context *)calloc(1, sizeof(fz_context));

    fz_pixmap *src = (fz_pixmap *)calloc(1, sizeof(fz_pixmap));
    fz_pixmap *dst = (fz_pixmap *)calloc(1, sizeof(fz_pixmap));
    fz_pixmap *shape = (fz_pixmap *)calloc(1, sizeof(fz_pixmap));

    // Set required fields; many are unused by the neutralized entry but kept sane
    src->w = 1; src->h = 1; src->n = 3; src->alpha = 0; src->stride = 3; src->x = 0; src->y = 0; src->s = 0;
    dst->w = 1; dst->h = 1; dst->n = 3; dst->alpha = 0; dst->stride = 3; dst->x = 0; dst->y = 0; dst->s = 0;
    shape->w = 1; shape->h = 1; shape->n = 1; shape->alpha = 0; shape->stride = 1; shape->x = 0; shape->y = 0; shape->s = 0;

    // Allocate sample buffers
    // Source has only 2 bytes to make sp[2] out-of-bounds in fz_blend_nonseparable_nonisolated
    unsigned char *src_buf = (unsigned char *)malloc(2);
    { static const unsigned char src_buf_data[] = {0x00, 0x00}; memcpy(src_buf, src_buf_data, (2 < sizeof(src_buf_data)) ? 2 : sizeof(src_buf_data)); };
    src->samples = src_buf;

    // Destination/shape buffers (sizes arbitrary but concrete); not dereferenced in our path
    unsigned char *dst_buf = (unsigned char *)malloc(4);
    { static const unsigned char dst_buf_data[] = {0x00, 0x00, 0x00, 0x00}; memcpy(dst_buf, dst_buf_data, (4 < sizeof(dst_buf_data)) ? 4 : sizeof(dst_buf_data)); };
    dst->samples = dst_buf;

    unsigned char *shape_buf = (unsigned char *)malloc(1);
    { static const unsigned char shape_buf_data[] = {0x00}; memcpy(shape_buf, shape_buf_data, (1 < sizeof(shape_buf_data)) ? 1 : sizeof(shape_buf_data)); };
    shape->samples = shape_buf;

    int alpha = 255;         // arbitrary
    int blendmode = 0;       // arbitrary
    int isolated = 0;        // arbitrary

    // Direct call into the neutralized entry (which unconditionally calls the vulnerable function)
    fz_blend_pixmap(ctx, dst, src, alpha, blendmode, isolated, shape);

    return 0;
}
