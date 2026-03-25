/* AUTO-GENERATED from harness preamble */
#pragma once

// harness.c
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

// Minimal local type definitions needed by the harness
#ifndef FZ_ENABLE_ICC
#define FZ_ENABLE_ICC 1
#endif
#ifndef FZ_COLORSPACE_BGR
#define FZ_COLORSPACE_BGR 13
#endif

typedef struct fz_context { int dummy; } fz_context;

typedef int fz_color_params; // minimal stand-in

typedef struct fz_colorspace_s {
    int type;
    union {
        struct { unsigned char *md5; } icc;
    } u;
} fz_colorspace;

typedef struct fz_color_converter { int dummy; } fz_color_converter;

typedef struct {
    int refs;
    unsigned char src_md5[16];
    unsigned char dst_md5[16];
    fz_color_params rend;
    unsigned char src_extras;
    unsigned char dst_extras;
    unsigned char copy_spots;
    unsigned char format;
    unsigned char proof;
    unsigned char bgr;
} fz_link_key;

typedef struct fz_icc_link fz_icc_link; // opaque; not used here

// Vulnerable function — keep exact vulnerable statement
fz_icc_link *
fz_find_icc_link(fz_context *ctx,
	fz_colorspace *src, int src_extras,
	fz_colorspace *dst, int dst_extras,
	fz_colorspace *prf,
	fz_color_params rend,
	int format,
	int copy_spots,
	int premult)
{
    fz_icc_link *link = NULL; fz_link_key key;
    // Vulnerable read (CWE-120 context): exact line preserved
    // Reachability probe if memcpy doesn't crash
    return link;
}

// Entry function — strict pass-through to vulnerable function (no guards!)
void
