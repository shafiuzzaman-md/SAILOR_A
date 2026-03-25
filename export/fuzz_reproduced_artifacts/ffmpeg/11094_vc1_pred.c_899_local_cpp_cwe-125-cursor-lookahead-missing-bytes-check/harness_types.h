/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdint.h>
#include <stdlib.h>

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

