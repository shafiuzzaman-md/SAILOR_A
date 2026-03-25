/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

// Minimal local type definitions to satisfy the signature
typedef struct { float x, y; } fz_point;
typedef struct { int x, y; } fz_ipoint;
typedef struct fz_context { int dummy; } fz_context;

typedef struct fz_pixmap {
    void *colorspace;
    int seps;
    int alpha;
    int xres, yres;
    int n;
    unsigned char *samples;
} fz_pixmap;

// Sliced vulnerable function: keep signature and ONLY the path needed to reach the sink
fz_pixmap *
