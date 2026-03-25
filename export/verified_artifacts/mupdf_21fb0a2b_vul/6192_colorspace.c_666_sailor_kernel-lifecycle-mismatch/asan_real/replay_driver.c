#include <string.h>
#include "harness_types.h"
#include <stdint.h>
#include <stdlib.h>
// klee removed for replay

// entry_func prototype from harness
int fz_default_output_intent(fz_context *ctx, const fz_default_colorspaces *default_cs);

int main() {
    // Allocate valid context and colorspace structures
    fz_context *ctx = (fz_context*)calloc(1, sizeof(fz_context));

    fz_default_colorspaces *dcs = (fz_default_colorspaces*)calloc(1, sizeof(fz_default_colorspaces));
    fz_colorspace *oi = (fz_colorspace*)calloc(1, sizeof(fz_colorspace));

    // Overapproximate: make contents symbolic, then fix pointers to valid allocations
    memset(dcs, 0, sizeof(*dcs)); /* replay: no ktest data for "dcs" */;
    memset(oi, 0, sizeof(*oi)); /* replay: no ktest data for "oi" */;

    dcs->oi = oi;  // ensure dereference is valid

    // Ensure non-NULL so we take the true branch in the ternary
    /* klee_assume removed */

    fz_default_output_intent(ctx, dcs);
    return 0;
}
