// Combined reproducer for 1600_mvs.c_1913_local_cpp_cwe-120-overflow
// Original harness: driver.c + smart_stubs.c + sliced source

// === smart_stubs.c ===
/* Smart stubs — auto-generated from path + vulnerability analysis */
/* Symbolic stubs model the environment: KLEE explores return values */
/* that both REACH the sink AND TRIGGER the vulnerability */
#include <stdlib.h>
#include <string.h>
#include <klee/klee.h>

/* PROACTIVE: function (auto-detected external) */
int function() { return 0; }

// === driver.c ===
// NO_HARNESS_TYPES
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <klee/klee.h>

#ifndef HMVP_MAX
#define HMVP_MAX 8
#endif

typedef struct { int16_t x, y; } MvField;

typedef struct EntryPoint {
    MvField hmvp[HMVP_MAX];
    int num_hmvp;
    MvField hmvp_ibc[HMVP_MAX];
    int num_hmvp_ibc;
} EntryPoint;

typedef struct { int min_pu_width; } PPS;

typedef struct {
    struct { PPS *pps; } ps;
    struct { MvField *mvf; } tab;
} VVCFrameContext;

typedef struct {
    int pred_mode;
    int cb_width, cb_height;
    int x0, y0;
} CodingUnit;

typedef struct VVCLocalContext {
    const VVCFrameContext *fc;
    const CodingUnit *cu;
    EntryPoint *ep;
} VVCLocalContext;

typedef struct MotionInfo { int dummy; } MotionInfo;

// Entry function from harness
void ff_vvc_update_hmvp(VVCLocalContext *lc, const MotionInfo *mi);

int main() {
    VVCLocalContext *lc = (VVCLocalContext *)calloc(1, sizeof(VVCLocalContext));
    VVCFrameContext *fc = (VVCFrameContext *)calloc(1, sizeof(VVCFrameContext));
    CodingUnit *cu = (CodingUnit *)calloc(1, sizeof(CodingUnit));
    EntryPoint *ep = (EntryPoint *)calloc(1, sizeof(EntryPoint));
    PPS *pps = (PPS *)calloc(1, sizeof(PPS));
    MotionInfo *mi = (MotionInfo *)calloc(1, sizeof(MotionInfo));

    lc->fc = fc;
    lc->cu = cu;
    lc->ep = ep;
    fc->ps.pps = pps;

    MvField *src = (MvField *)calloc(1, sizeof(MvField));
    klee_make_symbolic(src, sizeof(*src), "src_mvf");
    fc->tab.mvf = src;

    {
        MvField hmvp_tmp[HMVP_MAX];
        klee_make_symbolic(hmvp_tmp, sizeof(hmvp_tmp), "hmvp_buf");
        memcpy(ep->hmvp, hmvp_tmp, sizeof(hmvp_tmp));
        int num_hmvp_sym;
        klee_make_symbolic(&num_hmvp_sym, sizeof(num_hmvp_sym), "num_hmvp");
        ep->num_hmvp = num_hmvp_sym;
    }
    klee_assume(ep->num_hmvp >= HMVP_MAX);

    ff_vvc_update_hmvp(lc, mi);
    return 0;
}
