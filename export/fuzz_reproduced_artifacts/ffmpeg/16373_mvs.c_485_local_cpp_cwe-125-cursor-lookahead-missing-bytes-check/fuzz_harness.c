#include <stddef.h>
// Combined reproducer for 16373_mvs.c_485_local_cpp_cwe-125-cursor-lookahead-missing-bytes-check
// Original harness: driver.c + smart_stubs.c + sliced source

// === smart_stubs.c ===
/* Smart stubs — auto-generated from path + vulnerability analysis */
/* Symbolic stubs model the environment: KLEE explores return values */
/* that both REACH the sink AND TRIGGER the vulnerability */
#include <stdlib.h>
#include <string.h>
/* PROACTIVE: FUNCTION (auto-detected external) */
int FUNCTION() { return 0; }

/* PROACTIVE: function (auto-detected external) */
int function() { return 0; }

/* PROACTIVE: slice (auto-detected external) */
int slice() { return 0; }

// === driver.c ===
// NO_HARNESS_TYPES
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

int LLVMFuzzerTestOneInput(const uint8_t *fuzz_data, size_t fuzz_size) {
    if (fuzz_size < 64) return 0;
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

    memcpy(&log2_ctb_size, fuzz_data + (0), sizeof(log2_ctb_size));
    
    sps->log2_ctb_size = log2_ctb_size;

    memcpy(&log2_parallel_merge_level, fuzz_data + (sizeof(log2_ctb_size)), sizeof(log2_parallel_merge_level));
    pps->log2_parallel_merge_level = log2_parallel_merge_level;

    memcpy(&poc, fuzz_data + (sizeof(log2_ctb_size) + sizeof(log2_parallel_merge_level)), sizeof(poc));
    s->poc = poc;

    memcpy(&slice_type, fuzz_data + (sizeof(log2_ctb_size) + sizeof(log2_parallel_merge_level) + sizeof(poc)), sizeof(slice_type));
    s->sh.slice_type = slice_type;

    memcpy(&max_num_merge_cand, fuzz_data + (sizeof(log2_ctb_size) + sizeof(log2_parallel_merge_level) + sizeof(poc) + sizeof(slice_type)), sizeof(max_num_merge_cand));
    s->sh.max_num_merge_cand = max_num_merge_cand;

    memcpy(&cu_x, fuzz_data + (sizeof(log2_ctb_size) + sizeof(log2_parallel_merge_level) + sizeof(poc) + sizeof(slice_type) + sizeof(max_num_merge_cand)), sizeof(cu_x));
    memcpy(&cu_y, fuzz_data + (sizeof(log2_ctb_size) + sizeof(log2_parallel_merge_level) + sizeof(poc) + sizeof(slice_type) + sizeof(max_num_merge_cand) + sizeof(cu_x)), sizeof(cu_y));
    lc->cu.x = cu_x;
    lc->cu.y = cu_y;

    // Call arguments (locals are fine for klee_make_symbolic)
    int x0, y0, nPbW, nPbH, log2_cb_size, part_idx, merge_idx;
    memcpy(&x0, fuzz_data + (sizeof(log2_ctb_size) + sizeof(log2_parallel_merge_level) + sizeof(poc) + sizeof(slice_type) + sizeof(max_num_merge_cand) + sizeof(cu_x) + sizeof(cu_y)), sizeof(x0));
    memcpy(&y0, fuzz_data + (sizeof(log2_ctb_size) + sizeof(log2_parallel_merge_level) + sizeof(poc) + sizeof(slice_type) + sizeof(max_num_merge_cand) + sizeof(cu_x) + sizeof(cu_y) + sizeof(x0)), sizeof(y0));
    memcpy(&nPbW, fuzz_data + (sizeof(log2_ctb_size) + sizeof(log2_parallel_merge_level) + sizeof(poc) + sizeof(slice_type) + sizeof(max_num_merge_cand) + sizeof(cu_x) + sizeof(cu_y) + sizeof(x0) + sizeof(y0)), sizeof(nPbW));
    memcpy(&nPbH, fuzz_data + (sizeof(log2_ctb_size) + sizeof(log2_parallel_merge_level) + sizeof(poc) + sizeof(slice_type) + sizeof(max_num_merge_cand) + sizeof(cu_x) + sizeof(cu_y) + sizeof(x0) + sizeof(y0) + sizeof(nPbW)), sizeof(nPbH));
    memcpy(&log2_cb_size, fuzz_data + (sizeof(log2_ctb_size) + sizeof(log2_parallel_merge_level) + sizeof(poc) + sizeof(slice_type) + sizeof(max_num_merge_cand) + sizeof(cu_x) + sizeof(cu_y) + sizeof(x0) + sizeof(y0) + sizeof(nPbW) + sizeof(nPbH)), sizeof(log2_cb_size));
    
    memcpy(&part_idx, fuzz_data + (sizeof(log2_ctb_size) + sizeof(log2_parallel_merge_level) + sizeof(poc) + sizeof(slice_type) + sizeof(max_num_merge_cand) + sizeof(cu_x) + sizeof(cu_y) + sizeof(x0) + sizeof(y0) + sizeof(nPbW) + sizeof(nPbH) + sizeof(log2_cb_size)), sizeof(part_idx));

    memcpy(&merge_idx, fuzz_data + (sizeof(log2_ctb_size) + sizeof(log2_parallel_merge_level) + sizeof(poc) + sizeof(slice_type) + sizeof(max_num_merge_cand) + sizeof(cu_x) + sizeof(cu_y) + sizeof(x0) + sizeof(y0) + sizeof(nPbW) + sizeof(nPbH) + sizeof(log2_cb_size) + sizeof(part_idx)), sizeof(merge_idx));

    MvField out_mv;
    memset(&out_mv, 0, sizeof(out_mv));

    // Direct call
    entry_func(lc, pps, x0, y0, nPbW, nPbH, log2_cb_size, part_idx, merge_idx, &out_mv);

    return 0;
}
