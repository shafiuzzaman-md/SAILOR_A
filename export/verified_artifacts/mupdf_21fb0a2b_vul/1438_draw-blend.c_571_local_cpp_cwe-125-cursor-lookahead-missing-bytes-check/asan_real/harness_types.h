/* AUTO-GENERATED from harness preamble */
#pragma once

/* Minimal sliced harness for draw-blend.c vulnerability */
#include <stddef.h>
#include <stdint.h>

#ifndef FZ_RESTRICT
#define FZ_RESTRICT
#endif

typedef unsigned char byte;

typedef struct fz_context { int dummy; } fz_context;

typedef struct fz_pixmap {
    int w, h, n, stride, x, y, s;
    int alpha;               /* whether alpha channel present */
    unsigned char *samples;  /* pixel data */
    void *colorspace;        /* unused here */
} fz_pixmap;

/* Simple approximation of fz_mul255; exact semantics not needed for OOB */

/* Vulnerable function (neutralized to keep only the sink path). */
