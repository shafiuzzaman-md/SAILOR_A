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

int main() {
    fz_context *ctx = (fz_context *)malloc(sizeof(fz_context));
    fz_font *font = (fz_font *)malloc(sizeof(fz_font));
    if (!ctx || !font) return 0;

    { static const unsigned char ctx_data[] = {0x00, 0x00, 0x00, 0x00}; memcpy(ctx, ctx_data, (sizeof(*ctx) < sizeof(ctx_data)) ? sizeof(*ctx) : sizeof(ctx_data)); };
    { static const unsigned char font_data[] = {0x00, 0x00, 0x00, 0x00}; memcpy(font, font_data, (sizeof(*font) < sizeof(font_data)) ? sizeof(*font) : sizeof(font_data)); };

    fz_buffer *out = fz_extract_ttf_from_ttc(ctx, font);
    (void)out;
    return 0;
}
