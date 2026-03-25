/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stddef.h>
#include <string.h>

// Minimal type defs to satisfy the vulnerable function
typedef struct fz_context { int dummy; } fz_context;

typedef struct fz_bitmap {
    unsigned char *samples;
    int stride;
    int h;
    // other fields omitted
} fz_bitmap;

// Vulnerable function — keep the exact vulnerable statement, then add sink assertion
