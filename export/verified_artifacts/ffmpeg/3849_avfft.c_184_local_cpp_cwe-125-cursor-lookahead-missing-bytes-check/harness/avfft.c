#include <stddef.h>
#include <string.h>
#include <klee/klee.h>

// Minimal local type definitions to avoid external headers
typedef float FFTSample;
typedef struct RDFTContext RDFTContext;  // opaque external type

typedef void AVTXContext;  // opaque
typedef void (*av_tx_fn)(AVTXContext *ctx, float *dst, void *src, ptrdiff_t stride);

typedef struct AVTXWrapper {
    AVTXContext *ctx;
    av_tx_fn fn;

    AVTXContext *ctx2;
    av_tx_fn fn2;

    ptrdiff_t stride;
    int len;
    int inv;

    float *tmp;
    int out_of_place;
} AVTXWrapper;

// Vulnerable function (keep original body; insert sink assertion AFTER the vulnerable statement)
void av_rdft_calc(RDFTContext *s, FFTSample *data)
{
    AVTXWrapper *w = (AVTXWrapper *)s;
    float *src = w->inv ? w->tmp : (float *)data;
    float *dst = w->inv ? (float *)data : w->tmp;

    if (w->inv) {
        memcpy(src, data, w->len*sizeof(float));

        src[w->len] = src[1];
        klee_assert(0 && "SAILOR_SINK_REACHED");
        src[1] = 0.0f;
    }

    w->fn(w->ctx, dst, (void *)src, w->stride);

    if (!w->inv) {
        dst[1] = dst[w->len];
        memcpy(data, dst, w->len*sizeof(float));
    }
}

// Entry function: MUST be a direct pass-through to the vulnerable function
int entry_func(RDFTContext *s, FFTSample *data) {
    av_rdft_calc(s, data);
    return 0;
}
