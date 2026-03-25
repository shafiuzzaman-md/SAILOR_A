#include <stddef.h>
// Combined reproducer for 1162_psymodel.c_49_local_cpp_cwe-120-overflow
// Original harness: driver.c + smart_stubs.c + sliced source

// === smart_stubs.c ===
/* Smart stubs — auto-generated from path + vulnerability analysis */
/* Symbolic stubs model the environment: KLEE explores return values */
/* that both REACH the sink AND TRIGGER the vulnerability */
#include <stdlib.h>
#include <string.h>
/* PROACTIVE: array (auto-detected external) */
int array() { return 0; }

/* PROACTIVE: context (auto-detected external) */
int context() { return 0; }

/* PROACTIVE: function (auto-detected external) */
int function() { return 0; }

/* PROACTIVE: statements (auto-detected external) */
int statements() { return 0; }

// === driver.c ===
// NO_HARNESS_TYPES
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
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

int LLVMFuzzerTestOneInput(const uint8_t *fuzz_data, size_t fuzz_size) {
    if (fuzz_size < 64) return 0;
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
        memcpy(buf, fuzz_data + (0), 16);
        bands_src[i] = buf;
    }
    // Make the entire num_bands_src array symbolic in one call (required by KLEE)
    memcpy(num_bands_src, fuzz_data + (16), sizeof(*num_bands_src) * src_count);

    // num_lens is symbolic, force it to exceed destination capacity but within source bounds
    int num_lens;
    memcpy(&num_lens, fuzz_data + (16 + sizeof(*num_bands_src) * src_count), sizeof(num_lens));
    
    

    // Unused params in harness body, but pass valid concrete values
    int num_groups = 0;
    const uint8_t *group_map = (const uint8_t *)malloc(1);

    // Direct call to entry
    (void)entry_func(ctx, avctx, num_lens, bands_src, num_bands_src, num_groups, group_map);

    return 0;
}
