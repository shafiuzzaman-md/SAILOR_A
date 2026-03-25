/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifndef VP7_MVC_SIZE
#define VP7_MVC_SIZE 17
#endif
#ifndef VP8_MVC_SIZE
#define VP8_MVC_SIZE 19
#endif

// Minimal forward declarations to match entry/vul signatures
typedef struct AVFrame AVFrame;
typedef struct AVPacket AVPacket;

typedef struct AVCodecContext {
    void *priv_data;  // we only need priv_data for our path
} AVCodecContext;

// Minimal stand-in types to support the vulnerable memcpy statement
typedef struct FrameSlot {
    unsigned char bytes[64];
} FrameSlot;

typedef struct VP8Context {
    FrameSlot framep[4];
    FrameSlot next_framep[4];
} VP8Context;

// Vulnerable function (neutralized) — keep only the vulnerable statement verbatim
static int vp78_decode_frame(AVCodecContext *avctx, AVFrame *frame,
