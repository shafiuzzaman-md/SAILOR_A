/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifndef RLE_THRESHOLD
#define RLE_THRESHOLD 256
#endif

// Minimal type defs needed by the harness
typedef struct fz_context { int dummy; } fz_context;

typedef struct fz_pixmap {
    int x, y, w, h;
    int n;
    int stride;
    unsigned char *samples;
} fz_pixmap;

typedef struct fz_glyph {
    int x, y, w, h;
    int size;
    void *pixmap;
    unsigned char data[]; // flexible array as in real allocation pattern
} fz_glyph;

// Forward decl

// ENTRY: strict pass-through per rules
