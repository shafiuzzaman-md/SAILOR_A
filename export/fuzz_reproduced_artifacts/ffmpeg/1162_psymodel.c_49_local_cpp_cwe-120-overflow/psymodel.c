/* Minimal harness for ff_psy_init memcpy overflow at psymodel.c:49-50 */
#include <stdint.h>
#include <string.h>
#include <klee/klee.h>
#ifndef av_cold
#define av_cold
#endif

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
                        int num_groups, const uint8_t *group_map)
{
    // Keep ONLY the vulnerable statements
    (void)avctx; (void)num_groups; (void)group_map; // unused in harness

    memcpy(ctx->bands,     bands,     sizeof(ctx->bands[0])     *  num_lens);
    memcpy(ctx->num_bands, num_bands, sizeof(ctx->num_bands[0]) *  num_lens);

    // Universal sink assertion after the vulnerable statements
    klee_assert(0 && "SAILOR_SINK_REACHED");
    return 0;
}

// Entry function: MUST be a direct pass-through call to vul_func with no guards
int entry_func(FFPsyContext *ctx, AVCodecContext *avctx, int num_lens,
               const uint8_t **bands, const int* num_bands,
               int num_groups, const uint8_t *group_map) {
    return ff_psy_init(ctx, avctx, num_lens, bands, num_bands, num_groups, group_map);
}
