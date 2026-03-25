/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdlib.h>
#include <stdint.h>

// Minimal local definitions for required types/fields
#ifndef FZ_COLORSPACE_LAB
#define FZ_COLORSPACE_LAB 1
#endif
#ifndef FZ_COLORSPACE_INDEXED
#define FZ_COLORSPACE_INDEXED 2
#endif

typedef struct fz_context fz_context; // opaque placeholder

typedef struct fz_colorspace {
    int type;
    int n;
    union {
        struct {
            int high;
            unsigned char *lookup;
            struct fz_colorspace *base;
        } indexed;
    } u;
} fz_colorspace;

#ifndef fz_clamp
#define fz_clamp(a, lo, hi) (((a) < (lo)) ? (lo) : (((a) > (hi)) ? (hi) : (a)))
#endif

// Vulnerable function (kept verbatim from excerpt). Sink assertions added AFTER statements.
void
