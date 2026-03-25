#include <string.h>
// NO_HARNESS_TYPES
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
// klee removed for replay

// Minimal compatible type definitions matching harness/font.c
typedef struct fz_context_s { int dummy; } fz_context;
typedef struct fz_font_s { int dummy; } fz_font;
typedef struct fz_buffer_s { unsigned char *data; size_t size; } fz_buffer;

extern fz_buffer *fz_extract_ttf_from_ttc(fz_context *ctx, fz_font *font);

int LLVMFuzzerTestOneInput(const uint8_t *fuzz_data, size_t fuzz_size) {
    if (fuzz_size < 8) return 0;
    fz_context *ctx = (fz_context *)malloc(sizeof(fz_context));
    fz_font *font = (fz_font *)malloc(sizeof(fz_font));
    if (!ctx || !font) return 0;

    { memcpy(ctx, fuzz_data + 0, 4); };
    { memcpy(font, fuzz_data + 4, 4); };

    fz_buffer *out = fz_extract_ttf_from_ttc(ctx, font);
    (void)out;
    return 0;
}
