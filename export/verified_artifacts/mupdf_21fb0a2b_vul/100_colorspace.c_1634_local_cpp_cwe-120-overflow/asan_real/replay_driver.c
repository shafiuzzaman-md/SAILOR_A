#include "harness_types.h"
// klee removed for replay
#include <stdlib.h>
#include <string.h>

// Entry prototype from harness
void fz_convert_slow_pixmap_samples(fz_context *ctx, const fz_pixmap *src, fz_pixmap *dst, fz_colorspace *is, fz_color_params params, int copy_spots);

int main() {
    // Allocate core structs concretely
    fz_context *ctx = (fz_context*)calloc(1, sizeof(fz_context));
    fz_pixmap *src = (fz_pixmap*)calloc(1, sizeof(fz_pixmap));
    fz_pixmap *dst = (fz_pixmap*)calloc(1, sizeof(fz_pixmap));
    fz_colorspace *is = (fz_colorspace*)calloc(1, sizeof(fz_colorspace));
    fz_color_params params; memset(&params, 0, sizeof(params));

    // We bypassed entry guards in the harness, so we only need to drive the target branch.
    // Set a small concrete number of components so malloc sizes are concrete.
    dst->n = 4;                  // concrete, keeps malloc sizes concrete inside harness

    // Make spot count symbolic to allow KLEE to trigger OOB via dst_c/dst_s interplay.
    int sym_s; { static const unsigned char dst_s_data[] = {0x01, 0x00, 0x00, 0x00}; memcpy(&sym_s, dst_s_data, (sizeof(sym_s) < sizeof(dst_s_data)) ? sizeof(sym_s) : sizeof(dst_s_data)); };
    /* klee_assume removed */     // ensure we hit the memset(d + dst_c, 0, dst_s) path
    /* klee_assume removed */    // reasonable range to allow overflow
    dst->s = sym_s;

    // Ensure 'da' branch is taken; keep it simple/concrete
    dst->alpha = 1;              // da != 0 → enter the vulnerable branch

    // Other pixmap fields are unused in the harness path; keep them benign
    dst->w = 1; dst->h = 1; dst->stride = 4;
    src->w = 1; src->h = 1; src->stride = 4;

    // Drive the call chain directly
    fz_convert_slow_pixmap_samples(ctx, src, dst, is, params, 0);

    return 0;
}
