/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifndef PICT_FRAME
#define PICT_FRAME 3
#endif

// Minimal types to support the path
typedef struct H264Picture {
    int ref_count[2][2];
    int ref_poc[2][2][32];
    int mbaff;
    int poc;
} H264Picture;

typedef struct H264Context {
    int picture_structure;
    H264Picture *cur_pic_ptr;
} H264Context;

typedef struct H264SliceContext { int dummy; } H264SliceContext;

// Vulnerable function (neutralized to the minimal path containing the sink)
