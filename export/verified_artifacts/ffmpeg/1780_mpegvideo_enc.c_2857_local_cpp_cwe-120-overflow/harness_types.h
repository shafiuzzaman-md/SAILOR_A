/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <errno.h>

#ifndef AVERROR
#define AVERROR(e) (-(e))
#endif

// Minimal types to satisfy the vulnerable function
typedef struct AVCodecInternal {
    uint8_t *byte_buffer;
    int byte_buffer_size;
} AVCodecInternal;

typedef struct AVCodecContext {
    void *priv_data;
    AVCodecInternal *internal;
} AVCodecContext;

typedef struct PutBitContext {
    uint8_t *buf;
} PutBitContext;

typedef struct MpegEncContext {
    AVCodecContext *avctx;
    PutBitContext pb;
    int slice_context_count;
    uint8_t *ptr_lastgob;
    int chroma_y_shift;
    int width, height;
} MpegEncContext;

