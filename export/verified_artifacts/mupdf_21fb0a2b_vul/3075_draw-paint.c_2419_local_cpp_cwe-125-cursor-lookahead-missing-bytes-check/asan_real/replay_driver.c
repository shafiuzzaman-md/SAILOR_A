// NO_HARNESS_TYPES
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
// klee removed for replay

#ifndef FZ_RESTRICT
#define FZ_RESTRICT
#endif

typedef unsigned char byte;

typedef struct fz_pixmap {
    int n;
    int alpha;
    int x;
    int y;
    int stride;
    unsigned char *samples;
} fz_pixmap;

void fz_paint_pixmap_alpha(fz_pixmap * dst, const fz_pixmap * src, int alpha);

int main() {
    fz_pixmap *src = (fz_pixmap *)calloc(1, sizeof(fz_pixmap));
    fz_pixmap *dst = (fz_pixmap *)calloc(1, sizeof(fz_pixmap));

    size_t src_size = 1; // tiny buffer to stress sp += n-1
    size_t dst_size = 1;
    unsigned char *src_buf = (unsigned char *)malloc(src_size);
    unsigned char *dst_buf = (unsigned char *)malloc(dst_size);

    { static const unsigned char src_buf_data[] = {0x00}; memcpy(src_buf, src_buf_data, (src_size < sizeof(src_buf_data)) ? src_size : sizeof(src_buf_data)); };
    { static const unsigned char dst_buf_data[] = {0x00}; memcpy(dst_buf, dst_buf_data, (dst_size < sizeof(dst_buf_data)) ? dst_size : sizeof(dst_buf_data)); };

    // Initialize src
    src->samples = src_buf;
    src->n = 4;      // >=1, ensures sp += n-1 moves past buffer start
    src->alpha = 1;  // not used on our path but set sane
    src->x = 0;
    src->y = 0;
    src->stride = (int)src_size;

    // Initialize dst
    dst->samples = dst_buf;
    dst->n = 1;
    dst->alpha = 1;
    dst->x = 0;
    dst->y = 0;
    dst->stride = (int)dst_size;

    int alpha = 1; // non-zero to exercise non-solid path if used

    fz_paint_pixmap_alpha(dst, src, alpha);
    return 0;
}
