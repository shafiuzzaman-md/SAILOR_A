/* AUTO-GENERATED from harness preamble */
#pragma once

/* harness/spine.c - minimal neutralized harness for halftone.c:513 */
#include <stddef.h>
#include <stdint.h>

/* Minimal project types (sliced) */
typedef struct fz_context_s { int dummy; } fz_context;

typedef struct fz_pixmap_s {
    int w, h, n;            /* not used in slice */
    unsigned char *samples; /* pixel data */
} fz_pixmap;

typedef struct fz_halftone_s {
    int len;                 /* not used in slice */
    unsigned char *threshold;/* threshold line */
} fz_halftone;

typedef struct fz_bitmap_s {
    int w, h;                /* not used in slice */
    unsigned char *samples;  /* not used in slice */
} fz_bitmap;

/* ENTRY: direct pass-through — no guards, no checks */
