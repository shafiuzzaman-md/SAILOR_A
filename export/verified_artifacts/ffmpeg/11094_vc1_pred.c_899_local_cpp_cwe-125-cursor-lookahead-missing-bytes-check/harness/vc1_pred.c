#include <stdint.h>
#include <stdlib.h>
#include <klee/klee.h>

#ifndef MV_SIZE
#define MV_SIZE 8
#endif

#ifndef BMV_TYPE_BACKWARD
#define BMV_TYPE_BACKWARD 1
#endif
#ifndef BMV_TYPE_DIRECT
#define BMV_TYPE_DIRECT 2
#endif
#ifndef MB_TYPE_INTRA
#define MB_TYPE_INTRA 0x10
#endif

typedef struct Picture {
    int *mb_type;
    int16_t (*motion_val[2])[2];
} Picture;

typedef struct MpegEncContext {
    int mb_x, mb_y, mb_stride;
    int quarter_sample;
    int16_t mv[2][1][2];
    int block_index[4];
    Picture cur_pic;
    Picture next_pic;
    // storage for motion_val pointers (pointed to by cur_pic/next_pic)
    int16_t cur_mv0[MV_SIZE][2];
    int16_t cur_mv1[MV_SIZE][2];
    int16_t next_mv0[MV_SIZE][2];
    int16_t next_mv1[MV_SIZE][2];
    int mb_type_storage[MV_SIZE];
} MpegEncContext;

typedef struct VC1Context{
    MpegEncContext s;
    int bmvtype;
    int bfraction;
    int blocks_off;
    int mb_off;
    int cur_field_type;
    int ref_field_type[2];
} VC1Context;

// Neutral stub for scale_mv used by vulnerable statement
static inline int scale_mv(int mv, int bfraction, int dir, int quarter_sample) {
    // Overapproximate: return symbolic result
    int ret; klee_make_symbolic(&ret, sizeof(ret), "scale_mv_ret");
    return ret;
}

// Vulnerable function (neutralized to the target path). Keep the vulnerable line verbatim
void ff_vc1_pred_b_mv_intfi(VC1Context *v, int n, int *dmv_x, int *dmv_y,
                            int mv1, int *pred_flag)
{
    MpegEncContext *s = &v->s;
    // Neutralize guards/branches to directly execute the vulnerable path
    if (1) {
        // Vulnerable lookahead using s->block_index[0] + v->blocks_off
        s->mv[0][0][0] = scale_mv(s->next_pic.motion_val[1][s->block_index[0] + v->blocks_off][0],
                                  v->bfraction, 0, s->quarter_sample);
        // Reachability probe in case the statement doesn't crash
        klee_assert(0 && "SAILOR_SINK_REACHED");
    }
}

// Simple pass-through entry (not strictly needed, but provided per harness pattern)
int entry_func(VC1Context *v, int n, int *dmv_x, int *dmv_y, int mv1, int *pred_flag) {
    ff_vc1_pred_b_mv_intfi(v, n, dmv_x, dmv_y, mv1, pred_flag);
    return 0;
}
