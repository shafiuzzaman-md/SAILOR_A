// NO_HARNESS_TYPES
// klee removed for replay
#include <stdlib.h>
#include <string.h>

#ifndef FZ_MAX_COLORS
#define FZ_MAX_COLORS 32
#endif

/* Minimal matching types (must match harness/shade.c) */
typedef struct fz_context fz_context;
typedef struct fz_shade fz_shade;
typedef struct { float a, b, c, d, e, f; } fz_matrix;
typedef struct { float x0, y0, x1, y1; } fz_rect;
typedef struct fz_mesh_processor { int ncomp; } fz_mesh_processor;

typedef void fz_shade_prepare_fn(void);
typedef void fz_shade_process_fn(void);

/* Prototype of entry in harness */
void fz_process_shade(fz_context *ctx, fz_shade *shade, fz_matrix ctm, fz_rect scissor,
                      fz_shade_prepare_fn *prepare, fz_shade_process_fn *process, void *process_arg);

int main() {
    void *ctx_mem = malloc(256);
    void *shade_mem = malloc(256);
    if (!ctx_mem || !shade_mem) return 0;
    memset(ctx_mem, 0, 256);
    memset(shade_mem, 0, 256);

    fz_context *ctx = (fz_context *)ctx_mem;
    fz_shade *shade = (fz_shade *)shade_mem;

    fz_mesh_processor *painter = (fz_mesh_processor *)malloc(sizeof(*painter));
    if (!painter) return 0;
    memset(painter, 0, sizeof(*painter));

    { static const unsigned char ncomp_data[] = {0x00, 0x01, 0x00, 0x00}; memcpy(&painter->ncomp, ncomp_data, (sizeof(painter->ncomp) < sizeof(ncomp_data)) ? sizeof(painter->ncomp) : sizeof(ncomp_data)); };
    /* klee_assume removed */
    /* klee_assume removed */

    fz_matrix ctm; memset(&ctm, 0, sizeof(ctm));
    fz_rect scissor; memset(&scissor, 0, sizeof(scissor));

    fz_process_shade(ctx, shade, ctm, scissor, NULL, NULL, painter);
    return 0;
}
