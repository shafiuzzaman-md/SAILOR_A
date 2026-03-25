#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <klee/klee.h>

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

// Minimal allocator used by the vulnerable function
static int packet_alloc(AVBufferRef **pbuf, size_t size) {
    AVBufferRef *buf = (AVBufferRef*)malloc(sizeof(AVBufferRef));
    if (!buf) return -12; // -ENOMEM (approx)
    buf->buffer = NULL;
    buf->size = size;
    buf->data = (uint8_t*)malloc(size);
    if (!buf->data) { free(buf); return -12; }
    *pbuf = buf;
    return 0;
}

// VULNERABLE FUNCTION (keep original statement verbatim)
int av_packet_make_refcounted(AVPacket *pkt)
{
    int ret;

    if (pkt->buf)
        return 0;

    ret = packet_alloc(&pkt->buf, pkt->size);
    if (ret < 0)
        return ret;
    av_assert1(!pkt->size || pkt->data);
    if (pkt->size)
        memcpy(pkt->buf->data, pkt->data, pkt->size);

    // Reachability probe (fires only if memcpy didn't crash)
    klee_assert(0 && "SAILOR_SINK_REACHED");

    pkt->data = pkt->buf->data;

    return 0;
}

// ENTRY: pure pass-through (no guards)
int entry_func(AVPacket *pkt) {
    av_packet_make_refcounted(pkt);
    return 0;
}
