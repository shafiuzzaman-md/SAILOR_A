/* AUTO-GENERATED from harness preamble */
#pragma once


#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// Minimal local type shims to satisfy signatures
typedef struct { float a, b, c, d, e, f; } fz_matrix;
typedef struct { float x0, y0, x1, y1; } fz_rect;
typedef struct { int dummy; } fz_context;
typedef struct { int dummy; } xps_document;
typedef struct { int dummy; } xps_resource;
typedef struct { int dummy; } fz_xml;
typedef struct { int dummy; } fz_colorspace;

// Forward decls
void xps_parse_color(fz_context *ctx, xps_document *doc, char *base_uri, char *string,
                     fz_colorspace **csp, float *samples);

// Entry function: strict pass-through with NO guards and NO early returns
void xps_begin_opacity(fz_context *ctx, xps_document *doc, fz_matrix ctm, fz_rect area,
