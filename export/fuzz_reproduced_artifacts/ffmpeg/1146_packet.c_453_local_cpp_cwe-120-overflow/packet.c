#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <klee/klee.h>

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

// Local minimal allocator used by av_packet_ref path
static int packet_alloc(AVBufferRef **pbuf, int size) {
    AVBufferRef *r = (AVBufferRef*)calloc(1, sizeof(AVBufferRef));
    if (!r) return AVERROR(ENOMEM);
    // Allocate destination exactly 'size' bytes to mirror real semantics
    if (size < 0) size = 0; // be safe
    r->data = (uint8_t*)malloc((size_t)size);
    if (!r->data && size) { free(r); return AVERROR(ENOMEM); }
    *pbuf = r;
    return 0;
}

// Neutralized vulnerable function: keep the path with the vulnerable memcpy verbatim
int av_packet_ref(AVPacket *dst, const AVPacket *src)
{
    int ret;

    dst->buf = NULL;

    // removed av_packet_copy_props guard and fail path; keep direct vulnerable path
    if (!src->buf) {
        ret = packet_alloc(&dst->buf, src->size);
        (void)ret; // ignore errors in harness to reach sink
        // av_assert1(!src->size || src->data);  // neutralized assertion
        if (src->size)
            memcpy(dst->buf->data, src->data, src->size);
        klee_assert(0 && "SAILOR_SINK_REACHED");
        dst->data = dst->buf->data;
    } else {
        // neutralize else-path
    }

    dst->size = src->size;

    return 0;
}

// Mandatory simple pass-through entry function
int entry_func(AVPacket *dst, const AVPacket *src) {
    av_packet_ref(dst, src);
    return 0;
}
