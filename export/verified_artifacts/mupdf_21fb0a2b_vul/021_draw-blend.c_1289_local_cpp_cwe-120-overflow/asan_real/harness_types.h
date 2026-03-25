/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stddef.h>
#include <string.h>

#ifndef FZ_RESTRICT
#define FZ_RESTRICT
#endif

typedef unsigned char byte;

typedef struct fz_context {
    int dummy;
} fz_context;

typedef struct fz_pixmap {
    int n;
    int alpha;
    unsigned char *samples;
} fz_pixmap;

static inline void
