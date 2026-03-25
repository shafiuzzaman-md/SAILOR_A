/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifndef AV_INPUT_BUFFER_PADDING_SIZE
#define AV_INPUT_BUFFER_PADDING_SIZE 64
#endif

// Minimal types needed for the path to the sink
typedef struct AVCodecParameters {
    uint8_t *extradata;
    size_t   extradata_size;
} AVCodecParameters;

typedef struct AVCodecContext {
    uint8_t *extradata;
    size_t   extradata_size;
} AVCodecContext;

