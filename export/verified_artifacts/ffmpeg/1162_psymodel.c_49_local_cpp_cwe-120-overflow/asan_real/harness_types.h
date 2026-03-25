/* AUTO-GENERATED from harness preamble */
#pragma once

/* Minimal harness for ff_psy_init memcpy overflow at psymodel.c:49-50 */
#include <stdint.h>
#include <string.h>

// Minimal type definitions sufficient for this harness
typedef struct AVCodecContext {
    int dummy;  // Not used in the neutralized path
} AVCodecContext;

typedef struct FFPsyContext {
    AVCodecContext *avctx;            // encoder context (not used here)
    const void *model;                // not used
    void *ch;                         // not used
    void *group;                      // not used
    int num_groups;                   // not used
    int cutoff;                       // not used
    uint8_t **bands;                  // destination array (of pointers)
    int     *num_bands;               // destination array (of ints)
    int num_lens;                     // not used here
    struct { int size; int bits; int alloc; } bitres; // not used
    void* model_priv_data;            // not used
} FFPsyContext;

// Vulnerable function (neutralized to focus on the memcpy site)
av_cold int ff_psy_init(FFPsyContext *ctx, AVCodecContext *avctx, int num_lens,
                        const uint8_t **bands, const int* num_bands,
