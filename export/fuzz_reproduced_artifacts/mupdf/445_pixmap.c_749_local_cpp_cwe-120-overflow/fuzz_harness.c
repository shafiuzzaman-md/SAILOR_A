#include <stddef.h>
#include "harness_types.h"
// klee removed for replay
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

// Entry: fz_clear_pixmap_rect_with_value(ctx, dest, value, b)
extern void fz_clear_pixmap_rect_with_value(fz_context *ctx, fz_pixmap *dest, int value, fz_irect b);

int LLVMFuzzerTestOneInput(const uint8_t *fuzz_data, size_t fuzz_size) {
    if (fuzz_size < 32) return 0;
    // Concrete buffer sized deliberately too small for the chosen w*n to trigger overflow in memset
    const size_t BUF_SIZE = 32;      // 32 bytes total

    // Allocate context and pixmap
    fz_context ctx_obj; // contents unused by harness
    fz_context *ctx = &ctx_obj;

    fz_pixmap *dest = (fz_pixmap*)calloc(1, sizeof(fz_pixmap));
    if (!dest) return 0;

    unsigned char *buf = (unsigned char*)malloc(BUF_SIZE);
    if (!buf) return 0;
    // Make buffer bytes symbolic so KLEE sees symbolic memory but size is concrete
    { memcpy(buf, fuzz_data + 0, 32); };

    // Initialize pixmap fields
    dest->x = 0;
    dest->y = 0;
    dest->n = 4;            // 4 components per pixel
    dest->stride = 16;      // arbitrary; not used in first iteration (y=1)
    dest->samples = buf;    // base of pixel buffer
    dest->colorspace = 0;   // not used in harness

    // Rectangle parameters chosen to force overflow:
    // destp = samples + stride*(y0 - y) + (x0 - x)*n = samples + 0 + x0*n
    // Choose x0=7, n=4 -> offset = 28; choose w=2 -> memset length = 8 bytes -> 28+8 = 36 > 32 (overflow)
    fz_irect b;
    b.x0 = 7;
    b.x1 = 9;   // w = 2
    b.y0 = 0;
    b.y1 = 1;   // y = 1, single iteration

    int value = 255; // drive into the vulnerable branch

    // Call entry (direct pass-through per harness rules)
    fz_clear_pixmap_rect_with_value(ctx, dest, value, b);

    return 0;
}
