/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stddef.h>
#include <string.h>

#ifndef FZ_MAX_COMP
#define FZ_MAX_COMP 64
#endif

typedef struct fz_context { int dummy; } fz_context;
typedef struct fz_colorspace { int type; } fz_colorspace;
typedef struct fz_color_params { int dummy; } fz_color_params;

typedef struct fz_pixmap {
    int w;
    int h;
    int n;         // total comps (including spots + alpha)
    int s;         // number of spot components
    int stride;    // bytes per row
    int alpha;     // 0/1
    unsigned char *samples; // pixel buffer
    fz_colorspace *colorspace;
} fz_pixmap;

// Entry function: DIRECT pass-through to vulnerable function (no guards)

