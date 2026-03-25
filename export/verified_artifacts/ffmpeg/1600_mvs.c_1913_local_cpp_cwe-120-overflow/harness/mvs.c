#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <klee/klee.h>

// Minimal local definitions to satisfy signatures
#ifndef HMVP_MAX
#define HMVP_MAX 8
#endif

// Simplified MV/MVField types
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

typedef int (*compare_fn)(const MvField*, const MvField*);

// Dummy compare callbacks
static int compare_l0_mv(const MvField *a, const MvField *b) { (void)a; (void)b; return 0; }
static int compare_mv_ref_idx(const MvField *a, const MvField *b) { (void)a; (void)b; return 0; }

// Sliced vulnerable helper: keep the memmove sink
static void update_hmvp(MvField *hmvp, int *num_hmvp, const MvField *src, compare_fn cmp)
{
    (void)cmp; // neutralized path — comparator not used on the sink path
    int n = *num_hmvp;  // symbolic-controlled length
    if (n > 0) {
        // Vulnerable statement from the real pattern: unchecked length in memmove
        memmove(&hmvp[1], &hmvp[0], (size_t)n * sizeof(hmvp[0]));
        // Reachability probe — fires only if memmove didn't already crash
        klee_assert(0 && "SAILOR_SINK_REACHED");
    }
    hmvp[0] = *src;
    *num_hmvp = n + 1;
}

// ENTRY — must be a direct pass-through to the vulnerable function (no guards)
void ff_vvc_update_hmvp(VVCLocalContext *lc, const MotionInfo *mi)
{
    (void)mi; // unused in this slice
    // Direct call — no guards/early returns
    update_hmvp(lc->ep->hmvp, &lc->ep->num_hmvp, lc->fc->tab.mvf, compare_mv_ref_idx);
}
