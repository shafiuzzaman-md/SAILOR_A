#include <stdint.h>
#include <string.h>
// NO_HARNESS_TYPES
#include <stddef.h>
// klee removed for replay

// Minimal replicated types matching harness/colorspace.c

typedef struct fz_context { int dummy; } fz_context;

enum fz_colorspace_type {
    FZ_COLORSPACE_LAB = 1,
    FZ_COLORSPACE_INDEXED = 2,
    FZ_COLORSPACE_OTHER = 3
};

typedef struct { int high; } fz_colorspace_indexed_t;

typedef union { fz_colorspace_indexed_t indexed; } fz_colorspace_u_t;

typedef struct fz_colorspace {
    int n;
    enum fz_colorspace_type type;
    fz_colorspace_u_t u;
} fz_colorspace;

// Prototypes to avoid including stdlib.h
extern void *malloc(size_t);
extern void *calloc(size_t, size_t);

// Prototype for the target function
extern void fz_clamp_color(fz_context *ctx, fz_colorspace *cs, const float *in, float *out);

int LLVMFuzzerTestOneInput(const uint8_t *fuzz_data, size_t fuzz_size) {
    if (fuzz_size < 16) return 0;
    // Allocate context and colorspace
    fz_context *ctx = (fz_context *)calloc(1, sizeof(fz_context));
    fz_colorspace *cs = (fz_colorspace *)calloc(1, sizeof(fz_colorspace));

    // Force LAB branch (reads in[0], in[1], in[2])
    cs->type = FZ_COLORSPACE_LAB;
    cs->n = 3; // LAB components

    // Allocate an undersized input buffer (1 float) to trigger OOB on in[1]/in[2]
    float *in = (float *)malloc(sizeof(float) * 1);
    float *out = (float *)malloc(sizeof(float) * 3);

    // Make contents symbolic (sizes remain concrete)
    { memcpy(in, fuzz_data + 0, 4); };
    { memcpy(out, fuzz_data + 4, 12); };

    fz_clamp_color(ctx, cs, in, out);
    return 0;
}
