/* AUTO-GENERATED from harness preamble */
#pragma once

/* harness/spine.c */
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>

#ifndef MAX_LINE_SIZE
#define MAX_LINE_SIZE 1920
#endif

/* Minimal context with only the field we use */
typedef struct MpegvideoEncDSPContext {
    void (*draw_edges)(uint8_t *buf, ptrdiff_t wrap, int width, int height,
                       int w, int h, int sides);
} MpegvideoEncDSPContext;

/* Vulnerable implementation we bind in init (models real draw_edges bug path) */
