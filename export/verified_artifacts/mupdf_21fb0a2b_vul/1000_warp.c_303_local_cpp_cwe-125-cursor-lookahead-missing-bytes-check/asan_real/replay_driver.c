#include <string.h>
// NO_HARNESS_TYPES
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
// klee removed for replay

// Match the minimal types used in harness/warp.c
typedef struct { float x, y; } fz_point;
typedef struct { int x, y; } fz_ipoint;
typedef struct fz_context { int dummy; } fz_context;
typedef struct fz_pixmap {
    void *colorspace;
    int seps;
    int alpha;
    int xres, yres;
    int n;
    unsigned char *samples;
} fz_pixmap;

// Prototype of the target function
fz_pixmap *fz_warp_pixmap(fz_context *ctx, fz_pixmap *src, const fz_point points[4], int width, int height);

int main(void) {
    // Allocate concrete context and src (unused in sliced harness but safe)
    fz_context *ctx = (fz_context *)calloc(1, sizeof(fz_context));
    fz_pixmap *src = (fz_pixmap *)calloc(1, sizeof(fz_pixmap));

    // Prepare too-small points array to trigger OOB read in points[2]/points[3]
    fz_point pts2[2];
    { static const unsigned char pts2_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(&pts2, pts2_data, (sizeof(pts2) < sizeof(pts2_data)) ? sizeof(pts2) : sizeof(pts2_data)); };

    int width = 8, height = 8;

    (void)fz_warp_pixmap(ctx, src, pts2, width, height);
    return 0;
}
