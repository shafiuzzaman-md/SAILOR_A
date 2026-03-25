/* AUTO-GENERATED from harness preamble */
#pragma once

/* harness/spine.c */
#include <stdint.h>
#include <stddef.h>

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

