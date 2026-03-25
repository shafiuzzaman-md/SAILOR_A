/* AUTO-GENERATED from harness preamble */
#pragma once


// harness/encode_spine.c
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// Minimal local types to avoid heavy project headers
#ifndef AV_PKT_DATA_SIZE
#define AV_PKT_DATA_SIZE 64
#endif

typedef struct AVCodecContext {
    // Only keep fields we actually use in this slice
    unsigned char *internal_buffer;
    int internal_size;
} AVCodecContext;

typedef struct AVPacket {
    unsigned char *data;
    int size;
} AVPacket;

