/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stddef.h>
#include <string.h>

// Minimal local type definitions for harness
typedef struct fz_context { int dummy; } fz_context;

typedef struct fz_pixmap {
    int x;
    int y;
    int n;
    int stride;
    unsigned char *samples;
    void *colorspace;
} fz_pixmap;

typedef struct fz_irect {
    int x0, y0, x1, y1;
} fz_irect;

// Neutralized spine keeping the vulnerable statement verbatim
