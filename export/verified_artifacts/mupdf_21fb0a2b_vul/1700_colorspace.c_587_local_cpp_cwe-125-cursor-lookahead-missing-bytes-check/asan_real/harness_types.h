/* AUTO-GENERATED from harness preamble */
#pragma once

// Minimal harness for fz_clamp_color
#include <stddef.h>
#include <stdint.h>

// Minimal local definitions to satisfy the function signature/usage

typedef struct fz_context { int dummy; } fz_context;

enum fz_colorspace_type {
    FZ_COLORSPACE_LAB = 1,
    FZ_COLORSPACE_INDEXED = 2,
    FZ_COLORSPACE_OTHER = 3
};

typedef struct { int high; } fz_colorspace_indexed_t;

typedef union { fz_colorspace_indexed_t indexed; } fz_colorspace_u_t;

typedef struct fz_colorspace {
    int n;
    enum fz_colorspace_type type;
    fz_colorspace_u_t u;
} fz_colorspace;

#ifndef fz_clamp
#define fz_clamp(v, lo, hi) (( (v) < (lo) ) ? (lo) : ( ((v) > (hi)) ? (hi) : (v) ))
#endif

// Vulnerable function copied verbatim (with sink assertion after the risky deref)
void
