#include "harness_types.h"
// klee removed for replay
#include <stdlib.h>
#include <string.h>

int main() {
    // Context (unused in harness, but pass a valid pointer)
    fz_context ctx_obj; memset(&ctx_obj, 0, sizeof(ctx_obj));
    fz_context *ctx = &ctx_obj;

    // Destination pixmap: allocate ONLY 2 bytes to force OOB on bp[2]
    unsigned char *dst_buf = (unsigned char *)malloc(2);
    { static const unsigned char dst_buf_data[] = {0x00, 0x00}; memcpy(dst_buf, dst_buf_data, (2 < sizeof(dst_buf_data)) ? 2 : sizeof(dst_buf_data)); };

    // Source pixmap: allocate 3 bytes so sp[2] is in-bounds
    unsigned char *src_buf = (unsigned char *)malloc(3);
    { static const unsigned char src_buf_data[] = {0x00, 0x00, 0x00}; memcpy(src_buf, src_buf_data, (3 < sizeof(src_buf_data)) ? 3 : sizeof(src_buf_data)); };

    fz_pixmap dst; memset(&dst, 0, sizeof(dst));
    dst.w = 1; dst.h = 1; dst.n = 3; dst.stride = 3; dst.x = 0; dst.y = 0; dst.s = 0;
    dst.alpha = 0;               // avoid bp[3] access in harness
    dst.samples = dst_buf;
    dst.colorspace = NULL;

    fz_pixmap src; memset(&src, 0, sizeof(src));
    src.w = 1; src.h = 1; src.n = 3; src.stride = 3; src.x = 0; src.y = 0; src.s = 0;
    src.alpha = 0;               // avoid sp[3] access in harness
    src.samples = src_buf;
    src.colorspace = NULL;

    // Shape is unused by the harness entry, but pass a valid pointer anyway
    fz_pixmap shape; memset(&shape, 0, sizeof(shape));
    shape.samples = NULL; shape.w = shape.h = shape.n = shape.stride = 0;

    int alpha = 255;     // unused in harness
    int blendmode = 0;   // not used by harness path
    int isolated = 0;    // unused in harness

    // Call entry → directly invokes vulnerable function
    fz_blend_pixmap(ctx, &dst, &src, alpha, blendmode, isolated, &shape);

    return 0;
}
