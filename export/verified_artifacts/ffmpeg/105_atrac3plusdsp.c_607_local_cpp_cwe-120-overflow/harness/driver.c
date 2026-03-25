// NO_HARNESS_TYPES
#include <klee/klee.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#ifndef ATRAC3P_SUBBANDS
#define ATRAC3P_SUBBANDS 16
#endif
#ifndef ATRAC3P_SUBBAND_SAMPLES
#define ATRAC3P_SUBBAND_SAMPLES 16
#endif
#ifndef ATRAC3P_FRAME_SAMPLES
#define ATRAC3P_FRAME_SAMPLES 300
#endif

typedef struct AVTXContext { int dummy; } AVTXContext;
typedef void (*av_tx_fn)(AVTXContext *ctx, float *dst, const float *src, size_t stride);

typedef struct Atrac3pIPQFChannelCtx {
    int pos;
    float buf1[16][8];
    float buf2[16][8];
} Atrac3pIPQFChannelCtx;

int atrac3_entry(AVTXContext *dct_ctx, av_tx_fn dct_fn,
                 Atrac3pIPQFChannelCtx *hist, const float *in, float *out);

static void dummy_dct(AVTXContext *ctx, float *dst, const float *src, size_t stride) {
    (void)ctx; (void)dst; (void)src; (void)stride;
}

int main(void) {
    AVTXContext *dct_ctx = (AVTXContext *)calloc(1, sizeof(AVTXContext));
    Atrac3pIPQFChannelCtx *hist = (Atrac3pIPQFChannelCtx *)calloc(1, sizeof(Atrac3pIPQFChannelCtx));

    size_t in_elems = (size_t)ATRAC3P_SUBBANDS * (size_t)ATRAC3P_SUBBAND_SAMPLES;
    float *inbuf = (float *)malloc(in_elems * sizeof(float));
    klee_make_symbolic(inbuf, in_elems * sizeof(float), "inbuf");

    // Intentionally undersized output buffer to expose the overflow in memset
    size_t out_elems = 16; // smaller than ATRAC3P_FRAME_SAMPLES
    float *outbuf = (float *)malloc(out_elems * sizeof(float));
    klee_make_symbolic(outbuf, out_elems * sizeof(float), "outbuf");

    // Make hist symbolic
    klee_make_symbolic(hist, sizeof(*hist), "hist");

    atrac3_entry(dct_ctx, dummy_dct, hist, inbuf, outbuf);
    return 0;
}
