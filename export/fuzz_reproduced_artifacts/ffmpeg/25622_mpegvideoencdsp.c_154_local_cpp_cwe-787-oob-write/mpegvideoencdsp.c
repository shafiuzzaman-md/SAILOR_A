/* harness/spine.c */
#include <stdint.h>
#include <stddef.h>
#include <klee/klee.h>

#ifndef MAX_LINE_SIZE
#define MAX_LINE_SIZE 1920
#endif
#ifndef EDGE_WIDTH
#define EDGE_WIDTH 16
#endif

/* Opaque forward decl to satisfy signature */
typedef struct AVCodecContext AVCodecContext;

/* Minimal stand-in for context type used by entry signature */
typedef struct MpegvideoEncDSPContext {
    int (*try_8x8basis)(const int16_t rem[64], const int16_t weight[64],
                        const int16_t basis[64], int scale);
    void (*add_8x8basis)(int16_t rem[64], const int16_t basis[64], int scale);
    int (*pix_sum)(const uint8_t *pix, ptrdiff_t line_size);
    int (*pix_norm1)(const uint8_t *pix, ptrdiff_t line_size);
    void (*shrink[4])(uint8_t *dst, ptrdiff_t dst_wrap, const uint8_t *src,
                      ptrdiff_t src_wrap, int width, int height);
    void (*draw_edges)(uint8_t *buf, ptrdiff_t wrap, int width, int height,
                       int w, int h, int sides);
} MpegvideoEncDSPContext;

/* Modeled vulnerable function consistent with draw_edges signature */
void draw_edges_8_c(uint8_t *buf, ptrdiff_t wrap, int width, int height, int w, int h, int sides) {
    /* Vulnerable write pattern: computed offset can go out of bounds */
    ptrdiff_t idx = (ptrdiff_t)height * wrap + width + w;
    buf[idx] = 0; /* potential OOB write */
    /* Universal sink assertion AFTER the write */
    klee_assert(0 && "SAILOR_SINK_REACHED");
}

/* ENTRY: direct pass-through to the vulnerable function (no guards) */
void ff_mpegvideoencdsp_init(MpegvideoEncDSPContext *c, AVCodecContext *avctx) {
    (void)c; (void)avctx; /* unused in harness */
    extern uint8_t *g_buf;
    extern ptrdiff_t g_wrap;
    extern int g_width, g_height, g_w, g_h, g_sides;
    draw_edges_8_c(g_buf, g_wrap, g_width, g_height, g_w, g_h, g_sides);
}
