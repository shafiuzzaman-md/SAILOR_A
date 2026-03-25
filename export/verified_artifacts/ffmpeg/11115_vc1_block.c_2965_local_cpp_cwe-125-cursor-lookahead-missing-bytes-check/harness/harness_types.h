/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// Minimal types to support the sliced path

typedef struct {
    uint8_t *data[4];
} PictureLike;

typedef struct {
    // Fields used by the vulnerable statements
    PictureLike last_pic;
    uint8_t *dest[4];
    int linesize;
    int uvlinesize;
    int mb_y;
} MpegEncContext;

typedef struct VC1Context {
    MpegEncContext s;
} VC1Context;

// Sliced vulnerable function containing the real vulnerable statements
