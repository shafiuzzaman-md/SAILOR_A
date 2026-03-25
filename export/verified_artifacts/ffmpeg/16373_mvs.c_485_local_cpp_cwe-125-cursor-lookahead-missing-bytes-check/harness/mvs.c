/* harness/spine.c — neutralized slice for mvs.c target */
#include <klee/klee.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifndef XML_HIDDEN
#define XML_HIDDEN /* empty */
#endif

/* Locally define unknown structs/macros if needed */
struct _xmlDict { int seed; int size; void *table; void *subdict; int limit; };

/* Minimal type sketches extracted from observed source around target */
typedef struct Mv { int16_t x, y; } Mv;

typedef struct MvField {
    uint8_t pred_flag;
    Mv mv[2];
    int8_t ref_idx[2];
} MvField;

#ifndef PF_L0
#define PF_L0 1
#endif
#ifndef PF_L1
#define PF_L1 2
#endif
#ifndef PF_BI
#define PF_BI 3
#endif

#ifndef HEVC_SLICE_B
#define HEVC_SLICE_B 1
#endif

#ifndef MRG_MAX_NUM_CANDS
#define MRG_MAX_NUM_CANDS 5
#endif

#ifndef AV_ZERO32
#define AV_ZERO32(p) do { *(uint32_t*)(p) = 0; } while(0)
#endif

/* Forward minimal structs used by functions in mvs.c excerpt */

typedef struct HEVCSPS { int log2_ctb_size; int min_pu_width; } HEVCSPS;

typedef struct RefPicList { int list[16]; } RefPicList;

typedef struct HEVCFrame { const RefPicList *refPicList; const MvField *tab_mvf; } HEVCFrame;

typedef struct SliceHeader { int slice_type; int max_num_merge_cand; } SliceHeader;

typedef struct CodingUnit { int x; int y; } CodingUnit;

typedef struct HEVCLocalContext {
    struct HEVCContext *parent;
    CodingUnit cu;
} HEVCLocalContext;

typedef struct HEVCPPS { const HEVCSPS *sps; int log2_parallel_merge_level; } HEVCPPS;

typedef struct HEVCContext {
    int poc;
    const HEVCFrame *cur_frame;
    SliceHeader sh;
} HEVCContext;

/* Macros observed */
#ifndef TAB_MVF
#define TAB_MVF(x,y) tab_mvf[((y) * min_pu_width) + (x)]
#endif

/* Decls for helpers referenced in the slice (neutralized as no-ops) */
static void ff_hevc_set_neighbour_available(HEVCLocalContext *lc, int x0, int y0, int nPbW, int nPbH, int log2_ctb_size) { (void)lc; (void)x0; (void)y0; (void)nPbW; (void)nPbH; (void)log2_ctb_size; }
static void derive_spatial_merge_candidates(HEVCLocalContext *lc, const HEVCContext *s, const HEVCPPS *pps, const HEVCSPS *sps,
                                            int x0, int y0, int nPbW, int nPbH, int log2_cb_size, int singleMCLFlag, int part_idx,
                                            int merge_idx, MvField *mergecandlist)
{
    /* Provide at least one candidate so dereferences below are valid */
    klee_make_symbolic(mergecandlist, sizeof(MvField) * MRG_MAX_NUM_CANDS, "mergecandlist");
    (void)lc; (void)s; (void)pps; (void)sps; (void)x0; (void)y0; (void)nPbW; (void)nPbH; (void)log2_cb_size; (void)singleMCLFlag; (void)part_idx; (void)merge_idx;
}

static void mv_scale(Mv *dst, const Mv *src, int a, int b) { (void)a; (void)b; *dst = *src; }

/* ENTRY: simple pass-through to the vulnerable function (neutralized) */
int entry_func(HEVCLocalContext *lc, const HEVCPPS *pps,
               int x0, int y0, int nPbW, int nPbH, int log2_cb_size,
               int part_idx, int merge_idx, MvField *mv);

/* VULNERABLE FUNCTION (neutralized slice) — keep signature and target region */
void ff_hevc_luma_mv_merge_mode(HEVCLocalContext *lc, const HEVCPPS *pps,
                                int x0, int y0, int nPbW, int nPbH,
                                int log2_cb_size, int part_idx,
                                int merge_idx, MvField *mv)
{
    const HEVCSPS *const  sps = pps->sps;
    const HEVCContext *const s = lc->parent;
    int singleMCLFlag = 0;
    int nCS = 1 << log2_cb_size;
    MvField mergecand_list[MRG_MAX_NUM_CANDS];
    int nPbW2 = nPbW;
    int nPbH2 = nPbH;

    if (pps->log2_parallel_merge_level > 2 && nCS == 8) {
        singleMCLFlag = 1;
        x0 = lc->cu.x;
        y0 = lc->cu.y;
        nPbW = nCS;
        nPbH = nCS;
        part_idx = 0;
    }

    ff_hevc_set_neighbour_available(lc, x0, y0, nPbW, nPbH, sps->log2_ctb_size);
    derive_spatial_merge_candidates(lc, s, pps, sps, x0, y0, nPbW, nPbH, log2_cb_size,
                                    singleMCLFlag, part_idx, merge_idx, mergecand_list);

    /* Target site near reported line 485: read from candidate list using merge_idx */
    /* This models potential out-of-bounds read if merge_idx is not checked. */
    MvField tmp = mergecand_list[merge_idx];
    (void)nPbW2; (void)nPbH2;

    /* UNIVERSAL SINK ASSERTION: after the vulnerable statement */
    klee_assert(0 && "SAILOR_SINK_REACHED");

    *mv = tmp;
}

int entry_func(HEVCLocalContext *lc, const HEVCPPS *pps,
               int x0, int y0, int nPbW, int nPbH, int log2_cb_size,
               int part_idx, int merge_idx, MvField *mv)
{
    /* DIRECT call, no guards, as required */
    ff_hevc_luma_mv_merge_mode(lc, pps, x0, y0, nPbW, nPbH, log2_cb_size, part_idx, merge_idx, mv);
    return 0;
}
