/* harness/spine.c */
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <klee/klee.h>

#ifndef MAX_LINE_SIZE
#define MAX_LINE_SIZE 1920
#endif

/* Minimal context with only the field we use */
typedef struct MpegvideoEncDSPContext {
    void (*draw_edges)(uint8_t *buf, ptrdiff_t wrap, int width, int height,
                       int w, int h, int sides);
} MpegvideoEncDSPContext;

/* Vulnerable implementation we bind in init (models real draw_edges bug path) */
static void draw_edges_impl(uint8_t *buf, ptrdiff_t wrap, int width, int height,
                            int w, int h, int sides) {
    /* Model an out-of-bounds write that depends on wrap and height */
    /* If the driver allocates exactly wrap*height bytes, writing at index wrap*height is OOB by 1 */
    size_t idx = (size_t)(wrap * height);
    buf[idx] = 0xAA;  /* OOB write if buffer size == wrap*height */
    klee_assert(0 && "SAILOR_SINK_REACHED");
}

/* Public init to set function pointer, matching FFmpeg-style initializer name */
void ff_mpegvideoencdsp_init(MpegvideoEncDSPContext *c, void *avctx /* unused */) {
    (void)avctx;
    c->draw_edges = draw_edges_impl;
}

/* Neutralized entry: DIRECT call to the vulnerable function, no guards */
int entry_func(MpegvideoEncDSPContext *c, uint8_t *buf, ptrdiff_t wrap,
               int width, int height, int w, int h, int sides) {
    c->draw_edges(buf, wrap, width, height, w, h, sides);
    return 0;
}
