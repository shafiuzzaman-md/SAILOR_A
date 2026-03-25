/* AUTO-GENERATED from harness preamble */
#pragma once

/* Minimal sliced harness for MuPDF draw-paint.c vulnerability */
#include <stddef.h>
#include <stdint.h>

#ifndef FZ_RESTRICT
#define FZ_RESTRICT
#endif
#ifndef fz_forceinline
#define fz_forceinline
#endif
#ifndef TRACK_FN
#define TRACK_FN()
#endif

typedef unsigned char byte;

typedef struct fz_pixmap {
    int n;
    int alpha;
    int x;
    int y;
    int stride;
    unsigned char *samples;
} fz_pixmap;

/* Simple blend macro definitions sufficient for compilation */
#ifndef FZ_EXPAND
#define FZ_EXPAND(x) (x)
#endif
#ifndef FZ_COMBINE
#define FZ_COMBINE(s, a) (((int)(s) * (int)(a)) / 255)
#endif
#ifndef FZ_BLEND
#define FZ_BLEND(s, d, m) ((byte)((((int)(s) * (int)(m)) + ((int)(d) * (255 - (int)(m)))) / 255))
#endif

/* Vulnerable function — keep exact vulnerable statement and add sink assertion after it */
static fz_forceinline void
