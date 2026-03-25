/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifndef FZ_COLORSPACE_LAB
#define FZ_COLORSPACE_LAB 10
#endif

// Minimal project types used by the driver and harness
typedef struct fz_context { int dummy; } fz_context;

typedef struct fz_colorspace { int type; } fz_colorspace;

typedef struct fz_pixmap {
    int n;              // number of components including spots and alpha
    int s;              // number of spot colors
    int alpha;          // 1 if alpha present, else 0
    int w;              // width
    int h;              // height
    ptrdiff_t stride;   // bytes per line
    fz_colorspace *colorspace;
    unsigned char *samples; // data buffer (not used in this harness body)
} fz_pixmap;

typedef struct fz_color_params { int dummy; } fz_color_params;

// Forward decl of vulnerable function

// ENTRY FUNCTION — NEUTRALIZED pass-through (MANDATORY pattern)
