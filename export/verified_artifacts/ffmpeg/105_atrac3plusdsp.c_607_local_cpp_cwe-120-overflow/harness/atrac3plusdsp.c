/* Minimal harness for ff_atrac3p_ipqf overflow at atrac3plusdsp.c:607 */
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <klee/klee.h>

/* Project-agnostic local fallbacks (per system hint) */
#ifndef XML_HIDDEN
#define XML_HIDDEN /* empty */
#endif
struct _xmlDict { int seed; int size; void *table; void *subdict; int limit; };

/* Local macro definitions (reasonable guesses; concrete constants) */
#ifndef ATRAC3P_SUBBANDS
#define ATRAC3P_SUBBANDS 16
#endif
#ifndef ATRAC3P_SUBBAND_SAMPLES
#define ATRAC3P_SUBBAND_SAMPLES 16
#endif
#ifndef ATRAC3P_FRAME_SAMPLES
#define ATRAC3P_FRAME_SAMPLES 300
#endif

/* Minimal type stubs matching call sites */
typedef struct AVTXContext { int dummy; } AVTXContext;
typedef void (*av_tx_fn)(AVTXContext *ctx, float *dst, const float *src, size_t stride);

typedef struct Atrac3pIPQFChannelCtx {
    int pos;
    float buf1[16][8];
    float buf2[16][8];
} Atrac3pIPQFChannelCtx;

/* Vulnerable function — keep signature and the exact vulnerable statement */
void ff_atrac3p_ipqf(AVTXContext *dct_ctx, av_tx_fn dct_fn,
                     Atrac3pIPQFChannelCtx *hist, const float *in, float *out)
{
    /* Vulnerable memset from atrac3plusdsp.c:607 — MUST be verbatim */
    memset(out, 0, ATRAC3P_FRAME_SAMPLES * sizeof(*out));

    /* Universal sink assertion placed AFTER the vulnerable statement */
    klee_assert(0 && "SAILOR_SINK_REACHED");
}

/* Entry function: simple pass-through with no guards */
int atrac3_entry(AVTXContext *dct_ctx, av_tx_fn dct_fn,
                 Atrac3pIPQFChannelCtx *hist, const float *in, float *out)
{
    ff_atrac3p_ipqf(dct_ctx, dct_fn, hist, in, out);
    return 0;
}
