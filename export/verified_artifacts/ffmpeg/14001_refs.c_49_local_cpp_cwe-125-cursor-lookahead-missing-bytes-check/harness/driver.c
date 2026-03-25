#include "harness_types.h"
#include <klee/klee.h>
#include <stdint.h>
#include <stdlib.h>

#ifndef CTB_ARR_SZ
#define CTB_ARR_SZ 16
#endif

int entry_func(HEVCFrame *ref, int x0, int y0); // from harness/refs.c

int main() {
    // Allocate top-level structs concretely
    HEVCFrame *ref = (HEVCFrame *)calloc(1, sizeof(HEVCFrame));
    HEVCSPS  *sps  = (HEVCSPS *)calloc(1, sizeof(HEVCSPS));
    HEVCPPS  *pps  = (HEVCPPS *)calloc(1, sizeof(HEVCPPS));

    // Assign pointers
    ref->sps = sps;
    ref->pps = pps;

    // Concrete initialize SPS fields (no symbolic subfield on heap to avoid size issues)
    sps->log2_ctb_size = 0;  // minimal shift
    sps->ctb_width     = 2;  // small width

    // Allocate the ctb_addr_rs_to_ts array with CONCRETE size and fill deterministically
    int *ctb = (int *)malloc(sizeof(int) * CTB_ARR_SZ);
    pps->ctb_addr_rs_to_ts = ctb;
    for (int i = 0; i < CTB_ARR_SZ; ++i) ctb[i] = i;

    // x0, y0 symbolic inputs
    int x0, y0;
    klee_make_symbolic(&x0, sizeof(x0), "x0");
    klee_make_symbolic(&y0, sizeof(y0), "y0");
    // Constrain to non-negative reasonable range
    klee_assume(x0 >= 0);
    klee_assume(x0 < 4096);
    klee_assume(y0 >= 0);
    klee_assume(y0 < 4096);

    // Direct pass-through
    entry_func(ref, x0, y0);
    return 0;
}
