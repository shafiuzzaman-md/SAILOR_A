/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stddef.h>
#include <string.h>

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
