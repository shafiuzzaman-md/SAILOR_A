
// harness/spine.c
#include <stdint.h>
#include <stdlib.h>
#include <klee/klee.h>

// Minimal types to support the vulnerable line
typedef struct MpegEncContext {
    int mb_x, mb_y, mb_stride, mb_width;
    int first_slice_line;
    int linesize, uvlinesize;
    int *block_index;   // index array used by the vulnerable expression
    int *block_wrap;    // not used on the direct path but kept for type completeness
} MpegEncContext;

typedef struct VC1Context {
    MpegEncContext s;
    int fcm;
    int topleft_blk_idx, top_blk_idx, left_blk_idx, cur_blk_idx;
    uint8_t *mb_type[2];       // v->mb_type[0][...]
    uint8_t *fieldtx_plane;    // not used directly on the sink but present in signature area
    // blocks not needed for the sink
} VC1Context;

// Vulnerable function (neutralized): keep only the vulnerable statement verbatim
void ff_vc1_p_overlap_filter(VC1Context *v) {
    MpegEncContext *s = &v->s;
    int i = 0; // target the first block index

    // Vulnerable statement from vc1_loopfilter.c:178 (verbatim):
    if (v->mb_type[0][s->block_index[i]] && v->mb_type[0][s->block_index[i] - 1])
        ;

    // Universal sink assertion placed AFTER the vulnerable read
    klee_assert(0 && "SAILOR_SINK_REACHED");
}

// Entry function must be a pure pass-through to the vulnerable function
int harness_entry(VC1Context *v) {
    ff_vc1_p_overlap_filter(v);
    return 0;
}
