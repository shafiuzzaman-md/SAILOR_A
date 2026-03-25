/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifndef AVERROR
#define AVERROR(x) (-(x))
#endif
#ifndef ENOMEM
#define ENOMEM 12
#endif

// Minimal type definitions sufficient for the path
typedef struct AVBufferRef {
    uint8_t *data;
} AVBufferRef;

typedef struct AVPacket {
    AVBufferRef *buf;
    int64_t pts;
    int64_t dts;
    uint8_t *data;
    int size;
    int stream_index;
    int flags;
    void *side_data; // unused here
    int side_data_elems;
} AVPacket;

