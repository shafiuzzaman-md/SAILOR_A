/* AUTO-GENERATED from harness preamble */
#pragma once


// harness/spine.c
#include <stdint.h>
#include <stdlib.h>

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

