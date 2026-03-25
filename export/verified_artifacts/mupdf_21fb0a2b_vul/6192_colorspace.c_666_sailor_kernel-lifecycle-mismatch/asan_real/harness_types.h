/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stddef.h>
#include <stdint.h>

// Minimal type defs needed by the harness
typedef struct fz_context { int dummy; } fz_context;

typedef struct fz_colorspace {
    int type;
    int n;
} fz_colorspace;

typedef struct fz_default_colorspaces {
    fz_colorspace *gray;
    fz_colorspace *rgb;
    fz_colorspace *cmyk;
    fz_colorspace *oi;
} fz_default_colorspaces;

// Vulnerable function (keep vulnerable statement verbatim)
