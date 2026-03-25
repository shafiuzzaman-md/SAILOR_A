/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// Minimal type definitions sufficient for the harness
typedef struct fz_context { int dummy; } fz_context;

typedef struct fz_colorspace { int type; } fz_colorspace;

typedef struct fz_pixmap {
    int alpha;      // has alpha channel (da)
    int s;          // number of spot colorants (dst_s)
    int n;          // total number of components (dst_n)
    size_t w;
    int h;
    ptrdiff_t stride;
    struct fz_colorspace *colorspace;
    unsigned char *samples;
} fz_pixmap;

typedef struct fz_color_params { int dummy; } fz_color_params;

// Forward decl of vulnerable function

// ENTRY: must be a direct pass-through with no guards/early returns
