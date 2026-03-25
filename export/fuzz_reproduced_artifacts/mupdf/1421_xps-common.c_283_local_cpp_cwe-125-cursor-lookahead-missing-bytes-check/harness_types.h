/* AUTO-GENERATED from harness preamble */
#pragma once

/* auto-generated minimal harness spine */
#include <stddef.h>
#include <stdint.h>
#include <string.h>

/* Minimal type placeholders needed for signatures */
typedef struct fz_context { int dummy; } fz_context;

typedef struct fz_matrix { float m[6]; } fz_matrix;

typedef struct fz_rect { float x0, y0, x1, y1; } fz_rect;

typedef struct xps_resource { int dummy; } xps_resource;

typedef struct fz_xml { int dummy; } fz_xml;

typedef struct fz_colorspace { int dummy; } fz_colorspace;

typedef struct xps_document {
    void *dev;
    int opacity_top;
    float opacity[8];
    float *small_samples; /* driver-controlled small buffer to trigger OOB */
} xps_document;

/* Vulnerable function (neutralized, keep vulnerable statement verbatim) */
