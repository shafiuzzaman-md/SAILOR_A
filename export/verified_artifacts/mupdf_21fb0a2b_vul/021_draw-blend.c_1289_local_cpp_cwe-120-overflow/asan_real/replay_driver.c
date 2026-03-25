// NO_HARNESS_TYPES
// klee removed for replay
#include <stdlib.h>
#include <string.h>

#ifndef FZ_RESTRICT
#define FZ_RESTRICT
#endif

typedef unsigned char byte;

typedef struct fz_context { int dummy; } fz_context;
typedef struct fz_pixmap {
    int n;
    int alpha;
    unsigned char *samples;
} fz_pixmap;

// Prototype from harness
void fz_blend_pixmap_knockout(fz_context *ctx, fz_pixmap * FZ_RESTRICT dst, fz_pixmap * FZ_RESTRICT src, const fz_pixmap * FZ_RESTRICT shape);

int main() {
    fz_context *ctx = (fz_context*)calloc(1, sizeof(fz_context));
    fz_pixmap *src = (fz_pixmap*)calloc(1, sizeof(fz_pixmap));
    fz_pixmap *dst = (fz_pixmap*)calloc(1, sizeof(fz_pixmap));
    fz_pixmap *shape = (fz_pixmap*)calloc(1, sizeof(fz_pixmap));

    enum { SRC_SZ = 32, DST_SZ = 256, HP_SZ = 16 };
    byte *src_buf = (byte*)malloc(SRC_SZ);
    byte *dst_buf = (byte*)calloc(1, DST_SZ); // zeroed so bp[n1] == 0 when indexed
    byte *hp_buf  = (byte*)malloc(HP_SZ);

    // Symbolic contents
    { static const unsigned char src_buf_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(src_buf, src_buf_data, (SRC_SZ < sizeof(src_buf_data)) ? SRC_SZ : sizeof(src_buf_data)); };
    { static const unsigned char hp_buf_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(hp_buf, hp_buf_data, (HP_SZ < sizeof(hp_buf_data)) ? HP_SZ : sizeof(hp_buf_data)); };

    // Force knockout branch: ha == 0xFF, and let ba read in-bounds from dst
    hp_buf[0] = 0xFF;

    // Setup src with no alpha so sa = 255 and avoids sp[n1] pre-read
    int n;
    { static const unsigned char n_data[] = {0x21, 0x00, 0x00, 0x00}; memcpy(&n, n_data, (sizeof(n) < sizeof(n_data)) ? sizeof(n) : sizeof(n_data)); };
    // Constrain to keep bp[n1] in-bounds but memcpy length > SRC_SZ to overflow
    /* klee_assume removed */      // ensure memcpy reads past src buffer
    /* klee_assume removed */      // ensure bp[n1] access is in-bounds

    src->n = n;
    src->alpha = 0;               // sal = 0 -> sa = 255 (no sp[n1] read)
    src->samples = src_buf;

    dst->alpha = 1;               // bal != 0 so ba = bp[n1]
    dst->n = n;
    dst->samples = dst_buf;

    shape->samples = hp_buf;

    // Call entry (neutralized pass-through in harness)
    fz_blend_pixmap_knockout(ctx, dst, src, shape);
    return 0;
}
