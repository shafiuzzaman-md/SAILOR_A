#include <string.h>
#include "harness_types.h"
// klee removed for replay
#include <stdlib.h>

int main() {
    // Concrete allocations
    fz_context *ctx = (fz_context *)calloc(1, sizeof(fz_context));
    fz_shade *shade = (fz_shade *)calloc(1, sizeof(fz_shade));

    // Drive ncomp symbolically via a local, then assign into the struct
    int ncomp;
    { static const unsigned char ncomp_data[] = {0x40, 0x00, 0x00, 0x00}; memcpy(&ncomp, ncomp_data, (sizeof(ncomp) < sizeof(ncomp_data)) ? sizeof(ncomp) : sizeof(ncomp_data)); };
    /* klee_assume removed */
    /* klee_assume removed */
    shade->ncomp = ncomp;

    // Unused fields (neutralized entry ignores them)
    shade->type = 0;
    shade->use_function = 0;
    shade->colorspace = NULL;

    fz_matrix ctm = (fz_matrix){0};
    fz_rect scissor = (fz_rect){0};

    // Direct call into entry (which pass-throughs to type7 in the harness)
    fz_process_shade(ctx, shade, ctm, scissor, NULL, NULL, NULL);
    return 0;
}
