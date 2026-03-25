#include "harness_types.h"
#include <stdlib.h>
#include <string.h>
#include <klee/klee.h>

#ifndef BYTEBUF_SZ
#define BYTEBUF_SZ 128
#endif

int entry_func(MpegEncContext *s, size_t threshold, size_t size_increase);

int main() {
    // Allocate top-level context
    MpegEncContext *s = (MpegEncContext *)calloc(1, sizeof(MpegEncContext));

    // Allocate AVCodecContext and internal buffer holder
    AVCodecContext *avctx = (AVCodecContext *)calloc(1, sizeof(AVCodecContext));
    AVCodecInternal *aci = (AVCodecInternal *)calloc(1, sizeof(AVCodecInternal));

    // Concrete byte buffer with symbolic contents
    aci->byte_buffer = (uint8_t *)malloc(BYTEBUF_SZ);
    aci->byte_buffer_size = BYTEBUF_SZ;
    if (!s || !avctx || !aci || !aci->byte_buffer) return 0; // bail if OOM in concrete allocs

    klee_make_symbolic(aci->byte_buffer, BYTEBUF_SZ, "byte_buffer_data");

    // Wire up the context
    avctx->internal = aci;
    s->avctx = avctx;
    s->pb.buf = aci->byte_buffer;  // consistent with original code path
    s->slice_context_count = 1;    // typical value; not used in neutralized path

    // Arguments (concrete). Harness neutralizes guards; size_increase participates in min_size calc
    size_t threshold = 16;
    size_t size_increase = 32;

    // Call entry (must be direct pass-through to vulnerable function)
    entry_func(s, threshold, size_increase);

    return 0;
}
