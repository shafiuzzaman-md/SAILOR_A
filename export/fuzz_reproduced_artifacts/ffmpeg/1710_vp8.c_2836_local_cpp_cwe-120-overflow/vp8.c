#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <klee/klee.h>

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
    FrameSlot framep[1];
    FrameSlot next_framep[4];
} VP8Context;

// Vulnerable function (neutralized) — keep only the vulnerable statement verbatim
static int vp78_decode_frame(AVCodecContext *avctx, AVFrame *frame,
                             int *got_frame, AVPacket *avpkt, int is_vp8)
{
    VP8Context *s = (VP8Context*)avctx->priv_data;

    // Vulnerable statement from vp8.c around target line 2836 — VERBATIM
    memcpy(&s->framep[0], &s->next_framep[0], sizeof(s->framep[0]) * 4);
    klee_assert(0 && "SAILOR_SINK_REACHED");

    return 0;
}

// Entry function — MUST be a direct pass-through to the vulnerable function
int ff_vp8_decode_frame(AVCodecContext *avctx, AVFrame *frame,
                        int *got_frame, AVPacket *avpkt)
{
    return vp78_decode_frame(avctx, frame, got_frame, avpkt, 1);
}
