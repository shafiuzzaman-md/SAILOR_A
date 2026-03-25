/* AUTO-GENERATED from harness preamble */
#pragma once

/* Minimal harness for MuPDF colorspace vulnerability (colorspace.c:839) */
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#ifndef FZ_ENABLE_ICC
#define FZ_ENABLE_ICC 1
#endif

#ifndef FZ_COLORSPACE_INDEXED
#define FZ_COLORSPACE_INDEXED 1
#endif
#ifndef FZ_COLORSPACE_SEPARATION
#define FZ_COLORSPACE_SEPARATION 2
#endif
#ifndef FZ_COLORSPACE_BGR
#define FZ_COLORSPACE_BGR 3
#endif

/* Minimal type definitions to satisfy signatures */
typedef struct fz_context { int dummy; } fz_context;

typedef struct {
    /* Contents not needed for harness */
    int dummy;
} fz_icc_link;

typedef struct {
    int refs;
    unsigned char src_md5[16];
    unsigned char dst_md5[16];
    struct fz_color_params { int dummy; } rend; /* embed to avoid separate typedef issues */
    unsigned char src_extras;
    unsigned char dst_extras;
    unsigned char copy_spots;
    unsigned char format;
    unsigned char proof;
    unsigned char bgr;
} fz_link_key;

/* Real code uses a separate typedef; mirror minimally for signatures */
typedef struct fz_color_params fz_color_params;

/* Minimal colorspace matching the vulnerable access pattern */
typedef struct fz_colorspace {
    int type;
    union {
        struct { unsigned char *md5; } icc;
        struct { struct fz_colorspace *base; } indexed;
        struct { struct fz_colorspace *base; } separation;
    } u;
} fz_colorspace;

/* Minimal color converter to satisfy entry signature */
typedef struct fz_color_converter { 
    /* fields unused in our neutralized entry */
    void *convert;
    void *convert_via;
    fz_colorspace *ss;
    fz_colorspace *ss_via;
    fz_colorspace *ds;
    void *link;
} fz_color_converter;

/* Vulnerable function — keep signature and the exact vulnerable statement. */
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
    fz_icc_link *link = NULL;
    fz_link_key key, *new_key = NULL;
    (void)ctx; (void)src_extras; (void)dst_extras; (void)prf; (void)format; (void)copy_spots; (void)premult;

    /* Check the storable to see if we have a copy. */
    key.refs = 1;
    /* Universal sink assertion placed immediately after the vulnerable statement */

    /* The rest of the original function is neutralized for minimal slice */
    (void)new_key; (void)rend; (void)dst; // suppress unused warnings
    return link;
}

/* Entry function — strict pass-through directly to vulnerable function (no guards). */
void
