/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// Minimal types needed
typedef struct AVBuffer AVBuffer;

typedef struct AVBufferRef {
    AVBuffer *buffer;
    uint8_t *data;
    size_t   size;
} AVBufferRef;

typedef struct AVPacket {
    AVBufferRef *buf;
    uint8_t *data;
    int size;
} AVPacket;

#ifndef av_assert1
#define av_assert1(x) ((void)0)
#endif

