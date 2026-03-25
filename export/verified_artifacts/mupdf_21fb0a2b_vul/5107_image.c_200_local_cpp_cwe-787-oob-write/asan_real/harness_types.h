/* AUTO-GENERATED from harness preamble */
#pragma once

/* Minimal sliced harness for MuPDF image.c vulnerability
 * Spine: fz_decomp_image_from_stream -> fz_mask_color_key
 */
#include <stddef.h>
#include <stdint.h>

/* Minimal type stubs to satisfy signatures */
typedef struct fz_context { int dummy; } fz_context;
typedef struct fz_stream { int dummy; } fz_stream;
typedef struct fz_pixmap { int dummy; } fz_pixmap;
typedef struct fz_irect { int x0, y0, x1, y1; } fz_irect;

typedef struct fz_image {
    int n;           /* number of components */
    int bpc;         /* bits per component */
    int *colorkey;   /* pointer to color key table */
    int use_colorkey; /* flag */
} fz_image;

typedef struct fz_compressed_image {
    fz_image super;
    void *buffer;
} fz_compressed_image;

/* Vulnerable function — keep signature. Keep only the minimal path and the vulnerable statement verbatim. */
