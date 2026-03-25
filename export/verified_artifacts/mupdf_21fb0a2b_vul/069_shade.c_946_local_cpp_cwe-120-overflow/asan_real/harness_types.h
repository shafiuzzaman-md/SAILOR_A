/* AUTO-GENERATED from harness preamble */
#pragma once

/* Minimal sliced harness for shade.c type7 overflow */
#include <stddef.h>
#include <string.h>

#ifndef FZ_MAX_COLORS
#define FZ_MAX_COLORS 32
#endif

/* Minimal type stand-ins to satisfy signatures */
typedef struct fz_context fz_context;
typedef struct fz_shade fz_shade;
typedef struct { float a, b, c, d, e, f; } fz_matrix;
typedef struct { float x0, y0, x1, y1; } fz_rect;
typedef struct fz_mesh_processor { int ncomp; } fz_mesh_processor;
typedef void fz_shade_prepare_fn(void);
typedef void fz_shade_process_fn(void);

/* Forward decl of vulnerable function */

/* Entry function: DIRECT pass-through to vulnerable function, no guards */
void fz_process_shade(fz_context *ctx, fz_shade *shade, fz_matrix ctm, fz_rect scissor,
