#include <stdint.h>
#include <stddef.h>
// NO_HARNESS_TYPES
#include <stdlib.h>
#include <string.h>
// klee removed for replay

// Match the minimal types from harness/pixmap.c
typedef struct fz_context { int dummy; } fz_context;
typedef struct fz_colorspace { int dummy; } fz_colorspace;
typedef struct fz_pixmap {
    int x, y, w, h;
    int stride;
    unsigned char *samples;
    fz_colorspace *colorspace;
    void *seps;
    int alpha;
} fz_pixmap;

// entry_func is defined in harness/pixmap.c
extern int fz_clone_pixmap(fz_context *ctx, fz_pixmap *old);

int LLVMFuzzerTestOneInput(const uint8_t *fuzz_data, size_t fuzz_size) {
    if (fuzz_size < 512) return 0;
    fz_context *ctx = (fz_context *)calloc(1, sizeof(fz_context));
    fz_pixmap *old = (fz_pixmap *)calloc(1, sizeof(fz_pixmap));

    // Configure geometry so destination size is large (64*64*4 = 16384 bytes)
    old->x = 0;
    old->y = 0;
    old->w = 64;
    old->h = 64;
    old->alpha = 0;
    old->colorspace = NULL;
    old->seps = NULL;

    // Deliberately small source buffer to trigger memcpy over-read
    const size_t small = 1024; // << 16384
    unsigned char *src = (unsigned char *)malloc(small);
    { memcpy(src, fuzz_data + 0, 512); };
    old->samples = src;

    fz_clone_pixmap(ctx, old);
    return 0;
}
