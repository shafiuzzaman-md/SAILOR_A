/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// Minimal MuPDF type shims for harness
typedef struct fz_context_s { int dummy; } fz_context;

typedef struct { float x0, y0, x1, y1; } fz_rect;

typedef struct { float a, b, c, d, e, f; } fz_matrix;

// Font struct: only fields used along the path
typedef struct fz_font_s {
    int glyph_count;
    int use_glyph_bbox;
    fz_rect **bbox_table;
    fz_rect bbox;
    void *ft_face;
    void *t3lists;
} fz_font;

static const fz_rect fz_empty_rect = {0,0,0,0};

// MuPDF helpers (neutralized)
#ifndef Memento_label
#define Memento_label(ptr, str) (ptr)
#endif

#ifndef fz_malloc_array
#define fz_malloc_array(ctx, n, T) ((T *)calloc((size_t)(n), sizeof(T)))
#endif

// Vulnerable function (keep signature and core path)
static fz_rect *
