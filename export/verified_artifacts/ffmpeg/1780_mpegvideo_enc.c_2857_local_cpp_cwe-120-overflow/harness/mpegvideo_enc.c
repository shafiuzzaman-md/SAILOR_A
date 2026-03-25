#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <klee/klee.h>

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

// Stubs / helpers
static inline int put_bytes_left(PutBitContext *pb, int x) {
    (void)pb; (void)x;
    int ret; klee_make_symbolic(&ret, sizeof(ret), "put_bytes_left"); return ret;
}
static inline void emms_c(void) { }
static inline void av_free(void *p) { free(p); }
static inline void rebase_put_bits(PutBitContext *pb, uint8_t *buf, int size) { (void)size; pb->buf = buf; }

// av_fast_padded_malloc stub: allocate a SYMBOLIC-sized buffer and report its size
static inline void av_fast_padded_malloc(uint8_t **ptr, int *size, size_t min_size) {
    int newsz;
    klee_make_symbolic(&newsz, sizeof(newsz), "new_buffer_size");
    // Reasonable constraints: positive size, allow being smaller than min_size to expose overflow
    klee_assume(newsz > 0);
    klee_assume(newsz < (int)(min_size + 1024));
    uint8_t *mem = (uint8_t *)malloc((size_t)newsz);
    *ptr = mem;
    *size = mem ? newsz : 0;
}

// VULNERABLE FUNCTION (neutralized path) — keep exact sink line
int ff_mpv_reallocate_putbitbuffer(MpegEncContext *s, size_t threshold, size_t size_increase)
{
    (void)threshold; // neutralized
    // Directly execute the target path
    int lastgob_pos = 0;
    uint8_t *new_buffer = NULL;
    int new_buffer_size = 0;

    emms_c();

    // request at least old_size + increase (as in original)
    av_fast_padded_malloc(&new_buffer, &new_buffer_size,
                          (size_t)s->avctx->internal->byte_buffer_size + size_increase);
    if (!new_buffer)
        return AVERROR(ENOMEM);

    // EXACT vulnerable statement copied from source:
    memcpy(new_buffer, s->avctx->internal->byte_buffer, s->avctx->internal->byte_buffer_size);
    klee_assert(0 && "SAILOR_SINK_REACHED");

    av_free(s->avctx->internal->byte_buffer);
    s->avctx->internal->byte_buffer      = new_buffer;
    s->avctx->internal->byte_buffer_size = new_buffer_size;
    rebase_put_bits(&s->pb, new_buffer, new_buffer_size);
    s->ptr_lastgob   = s->pb.buf + lastgob_pos;

    return 0;
}

// ENTRY: mandatory direct pass-through
int entry_func(MpegEncContext *s, size_t threshold, size_t size_increase) {
    ff_mpv_reallocate_putbitbuffer(s, threshold, size_increase);
    return 0;
}
