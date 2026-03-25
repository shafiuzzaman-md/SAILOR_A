// NO_HARNESS_TYPES
// klee removed for replay
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

// Minimal replicas matching harness/font.c
typedef struct fz_context_s { int dummy; } fz_context;
typedef struct { float x0, y0, x1, y1; } fz_rect;
typedef struct { float a, b, c, d, e, f; } fz_matrix;
typedef struct fz_font_s {
    int glyph_count;
    int use_glyph_bbox;
    fz_rect **bbox_table;
    fz_rect bbox;
    void *ft_face;
    void *t3lists;
} fz_font;

// Prototype from harness
fz_rect fz_bound_glyph(fz_context *ctx, fz_font *font, int gid, fz_matrix trm);

int main() {
    fz_context *ctx = (fz_context *)calloc(1, sizeof(fz_context));
    fz_font *font = (fz_font *)calloc(1, sizeof(fz_font));

    // Initialize fields to reach get_gid_bbox allocation path
    font->bbox_table = NULL;
    font->use_glyph_bbox = 1;
    font->ft_face = NULL;
    font->t3lists = NULL;

    int glyph_count;
    { static const unsigned char glyph_count_data[] = {0x01, 0x00, 0x00, 0x00}; memcpy(&glyph_count, glyph_count_data, (sizeof(glyph_count) < sizeof(glyph_count_data)) ? sizeof(glyph_count) : sizeof(glyph_count_data)); };
    /* klee_assume removed */
    /* klee_assume removed */
    font->glyph_count = glyph_count;

    int gid;
    { static const unsigned char gid_data[] = {0x00, 0x00, 0x00, 0x00}; memcpy(&gid, gid_data, (sizeof(gid) < sizeof(gid_data)) ? sizeof(gid) : sizeof(gid_data)); };
    /* klee_assume removed */
    /* klee_assume removed */

    fz_matrix trm;
    memset(&trm, 0, sizeof(trm)); /* replay: no ktest data for "trm" */;

    (void)fz_bound_glyph(ctx, font, gid, trm);
    return 0;
}
