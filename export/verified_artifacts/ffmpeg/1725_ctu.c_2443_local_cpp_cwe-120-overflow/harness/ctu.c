#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <klee/klee.h>

#ifndef VVC_MAX_REF_ENTRIES
#define VVC_MAX_REF_ENTRIES 8
#endif

// Minimal project-specific structs to reach the vulnerable site
typedef struct CTU {
    int has_dmvr;
    int max_y[2][VVC_MAX_REF_ENTRIES];
} CTU;

typedef struct VVCFrameTab {
    CTU *ctus;
    const void **cus;  // unused in our slice
} VVCFrameTab;

typedef struct VVCFrameContext {
    VVCFrameTab tab;
} VVCFrameContext;

typedef struct H266RawSliceHeader {
    int num_ref_idx_active[2];
} H266RawSliceHeader;

typedef struct SliceHeaderWrap {
    H266RawSliceHeader *r;
} SliceHeaderWrap;

typedef struct SliceContext {
    SliceHeaderWrap sh;
} SliceContext;

typedef struct VVCLocalContext {
    VVCFrameContext *fc;
    SliceContext *sc;
} VVCLocalContext;

// Stub of IS_I controlling the guard; keep symbolic but force non-intra to reach the sink
static int IS_I(const H266RawSliceHeader *rsh) {
    int v;
    klee_make_symbolic(&v, sizeof(v), "IS_I_ret");
    klee_assume(v == 0); // ensure we do not early-return
    return v;
}

// Vulnerable function slice — keep the exact vulnerable statement and add sink assertion after it
static void ctu_get_pred(VVCLocalContext *lc, const int rs)
{
    const VVCFrameContext *fc       = lc->fc;
    const H266RawSliceHeader *rsh   = lc->sc->sh.r;
    CTU *ctu                        = fc->tab.ctus + rs;

    ctu->has_dmvr = 0;

    if (IS_I(rsh))
        return;

    for (int lx = 0; lx < 2; lx++)
        memset(ctu->max_y[lx], -1, sizeof(ctu->max_y[0][0]) * rsh->num_ref_idx_active[lx]);

    // Universal sink assertion — after the vulnerable statement
    klee_assert(0 && "SAILOR_SINK_REACHED");
}

// Entry function — strict pass-through to the vulnerable function
int ff_vvc_coding_tree_unit(VVCLocalContext *lc,
    const int ctu_idx, const int rs, const int rx, const int ry)
{
    (void)ctu_idx; (void)rx; (void)ry; // unused in slice
    ctu_get_pred(lc, rs);
    return 0;
}
