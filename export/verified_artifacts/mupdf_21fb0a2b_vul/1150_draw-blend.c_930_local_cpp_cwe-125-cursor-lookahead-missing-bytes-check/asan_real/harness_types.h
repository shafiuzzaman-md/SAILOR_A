/* AUTO-GENERATED from harness preamble */
#pragma once

/* harness/spine.c */
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#ifndef FZ_RESTRICT
#define FZ_RESTRICT
#endif

typedef unsigned char byte;

typedef struct fz_context {
    int dummy;
} fz_context;

typedef struct fz_pixmap {
    int w, h;           // width, height
    int n;              // number of components (including alpha)
    int alpha;          // alpha components (0 or 1)
    int stride;         // bytes per row
    int x, y;           // origin
    int s;              // spot colors
    unsigned char *samples; // sample buffer
    void *colorspace;   // colorspace pointer
} fz_pixmap;

/* Vulnerable function — keep only the minimal path with the exact vulnerable statement */
