#include "harness_types.h"
#include <stdlib.h>
#include <string.h>
#include <klee/klee.h>

int main() {
    // Concrete allocations
    VVCLocalContext *lc = (VVCLocalContext*)calloc(1, sizeof(VVCLocalContext));
    VVCFrameContext *fc = (VVCFrameContext*)calloc(1, sizeof(VVCFrameContext));
    SliceContext *sc = (SliceContext*)calloc(1, sizeof(SliceContext));
    H266RawSliceHeader *rsh = (H266RawSliceHeader*)calloc(1, sizeof(H266RawSliceHeader));

    // Wire pointers
    lc->fc = fc;
    lc->sc = sc;
    sc->sh.r = rsh;

    // Drive path: non-intra ensured by IS_I stub (returns 0 via assume)
    // Set large ref counts to stress memset size
    rsh->num_ref_idx_active[0] = 32;
    rsh->num_ref_idx_active[1] = 32;

    // Allocate CTU table for rs=0
    fc->tab.ctus = (CTU*)calloc(1, sizeof(CTU) * 1);

    // Call entry (strict pass-through to vulnerable slice)
    ff_vvc_coding_tree_unit(lc, 0, 0, 0, 0);
    return 0;
}
