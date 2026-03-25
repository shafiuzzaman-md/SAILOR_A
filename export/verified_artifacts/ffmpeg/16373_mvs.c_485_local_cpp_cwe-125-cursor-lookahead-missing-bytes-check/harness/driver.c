// NO_HARNESS_TYPES
#include <klee/klee.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct Mv { int16_t x, y; } Mv;

typedef struct MvField {
    uint8_t pred_flag;
    Mv mv[2];
    int8_t ref_idx[2];
} MvField;

typedef struct HEVCSPS { int log2_ctb_size; int min_pu_width; } HEVCSPS;

typedef struct RefPicList { int list[16]; } RefPicList;

typedef struct HEVCFrame { const RefPicList *refPicList; const MvField *tab_mvf; } HEVCFrame;

typedef struct SliceHeader { int slice_type; int max_num_merge_cand; } SliceHeader;

typedef struct CodingUnit { int x; int y; } CodingUnit;

typedef struct HEVCContext {
    int poc;
    const HEVCFrame *cur_frame;
    SliceHeader sh;
} HEVCContext;

typedef struct HEVCPPS { const HEVCSPS *sps; int log2_parallel_merge_level; } HEVCPPS;

typedef struct HEVCLocalContext {
    HEVCContext *parent;
    CodingUnit cu;
} HEVCLocalContext;

extern int entry_func(HEVCLocalContext *lc, const HEVCPPS *pps,
               int x0, int y0, int nPbW, int nPbH, int log2_cb_size,
               int part_idx, int merge_idx, MvField *mv);

int main() {
    // Concrete allocations
    HEVCLocalContext *lc = (HEVCLocalContext *)calloc(1, sizeof(HEVCLocalContext));
    HEVCPPS *pps = (HEVCPPS *)calloc(1, sizeof(HEVCPPS));
    HEVCSPS *sps = (HEVCSPS *)calloc(1, sizeof(HEVCSPS));
    HEVCContext *s = (HEVCContext *)calloc(1, sizeof(HEVCContext));

    // Wire pointers
    pps->sps = sps;
    lc->parent = s;

    // Make locals symbolic, then assign into struct fields
    int log2_ctb_size, log2_parallel_merge_level, poc, slice_type, max_num_merge_cand;
    int cu_x, cu_y;

    klee_make_symbolic(&log2_ctb_size, sizeof(log2_ctb_size), "log2_ctb_size");
    klee_assume(log2_ctb_size >= 0 && log2_ctb_size <= 6);
    sps->log2_ctb_size = log2_ctb_size;

    klee_make_symbolic(&log2_parallel_merge_level, sizeof(log2_parallel_merge_level), "log2_parallel_merge_level");
    pps->log2_parallel_merge_level = log2_parallel_merge_level;

    klee_make_symbolic(&poc, sizeof(poc), "poc");
    s->poc = poc;

    klee_make_symbolic(&slice_type, sizeof(slice_type), "slice_type");
    s->sh.slice_type = slice_type;

    klee_make_symbolic(&max_num_merge_cand, sizeof(max_num_merge_cand), "max_num_merge_cand");
    s->sh.max_num_merge_cand = max_num_merge_cand;

    klee_make_symbolic(&cu_x, sizeof(cu_x), "cu_x");
    klee_make_symbolic(&cu_y, sizeof(cu_y), "cu_y");
    lc->cu.x = cu_x;
    lc->cu.y = cu_y;

    // Call arguments (locals are fine for klee_make_symbolic)
    int x0, y0, nPbW, nPbH, log2_cb_size, part_idx, merge_idx;
    klee_make_symbolic(&x0, sizeof(x0), "x0");
    klee_make_symbolic(&y0, sizeof(y0), "y0");
    klee_make_symbolic(&nPbW, sizeof(nPbW), "nPbW");
    klee_make_symbolic(&nPbH, sizeof(nPbH), "nPbH");
    klee_make_symbolic(&log2_cb_size, sizeof(log2_cb_size), "log2_cb_size");
    klee_assume(log2_cb_size >= 0 && log2_cb_size <= 6);
    klee_make_symbolic(&part_idx, sizeof(part_idx), "part_idx");

    klee_make_symbolic(&merge_idx, sizeof(merge_idx), "merge_idx");

    MvField out_mv;
    memset(&out_mv, 0, sizeof(out_mv));

    // Direct call
    entry_func(lc, pps, x0, y0, nPbW, nPbH, log2_cb_size, part_idx, merge_idx, &out_mv);

    return 0;
}
