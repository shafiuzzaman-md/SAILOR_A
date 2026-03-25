/* AUTO-GENERATED from harness preamble */
#pragma once

#include <assert.h>
#include <stdlib.h>
#include <string.h>

// Minimal local type defs sufficient for the slice
typedef struct fz_context fz_context; // opaque

typedef struct fz_pixmap {
    int n;              // number of components
    int w, h;           // width, height
    int stride;         // bytes per row
    unsigned char *samples; // pixel data
} fz_pixmap;

// Neutralized vulnerable function (entry == vulnerable)
// Keep signature and the vulnerable statement verbatim. Allocate locally.
fz_pixmap *
