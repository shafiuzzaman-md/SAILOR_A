// NO_HARNESS_TYPES
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <klee/klee.h>

#ifndef av_cold
#define av_cold
#endif

typedef struct AVCodecContext {
    int dummy;  // not used by the harnessed path
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

int entry_func(FFPsyContext *ctx, AVCodecContext *avctx, int num_lens,
               const uint8_t **bands, const int* num_bands,
               int num_groups, const uint8_t *group_map);

int main() {
    // Allocate context structs
    FFPsyContext *ctx = (FFPsyContext *)calloc(1, sizeof(FFPsyContext));
    AVCodecContext *avctx = (AVCodecContext *)calloc(1, sizeof(AVCodecContext));

    // Destination arrays (small) in ctx to trigger overflow when num_lens is large
    const int dst_count = 4; // small capacity
    ctx->bands = (uint8_t **)malloc(sizeof(ctx->bands[0]) * dst_count);
    ctx->num_bands = (int *)malloc(sizeof(ctx->num_bands[0]) * dst_count);

    // Initialize destination arrays to non-NULL to avoid undefined reads
    for (int i = 0; i < dst_count; i++) {
        ctx->bands[i] = (uint8_t *)malloc(8); // small buffers
        memset(ctx->bands[i], 0xAA, 8);
        ctx->num_bands[i] = 0;
    }

    // Source arrays (larger) to support bigger num_lens without read OOB
    const int src_count = 16;
    const uint8_t **bands_src = (const uint8_t **)malloc(sizeof(bands_src[0]) * src_count);
    int *num_bands_src = (int *)malloc(sizeof(num_bands_src[0]) * src_count);

    for (int i = 0; i < src_count; i++) {
        uint8_t *buf = (uint8_t *)malloc(16);
        klee_make_symbolic(buf, 16, "bands_src_buf");
        bands_src[i] = buf;
    }
    // Make the entire num_bands_src array symbolic in one call (required by KLEE)
    klee_make_symbolic(num_bands_src, sizeof(*num_bands_src) * src_count, "num_bands_src");

    // num_lens is symbolic, force it to exceed destination capacity but within source bounds
    int num_lens;
    klee_make_symbolic(&num_lens, sizeof(num_lens), "num_lens");
    klee_assume(num_lens > dst_count);
    klee_assume(num_lens <= src_count);

    // Unused params in harness body, but pass valid concrete values
    int num_groups = 0;
    const uint8_t *group_map = (const uint8_t *)malloc(1);

    // Direct call to entry
    (void)entry_func(ctx, avctx, num_lens, bands_src, num_bands_src, num_groups, group_map);

    return 0;
}
