#include "harness_types.h"
#include <stdlib.h>
#include <stdint.h>
#include <klee/klee.h>

int main() {
    // Initialize context and function pointers
    MpegvideoEncDSPContext c = {0};
    ff_mpegvideoencdsp_init(&c, NULL);

    // Concrete dimensions (ensure buffer larger than MAX_LINE_SIZE)
    ptrdiff_t wrap = 64;   // bytes per line
    int height = 100;      // lines
    int width = 64;        // unused in impl but provided
    int w = 16;
    int h = 16;
    int sides = 0xF;       // arbitrary flags

    size_t buf_size = (size_t)(wrap * height); // exact size so write at index==size is OOB by 1
    uint8_t *buf = (uint8_t *)malloc(buf_size);
    if (!buf) return 0;

    // Make buffer contents symbolic for exploration
    klee_make_symbolic(buf, buf_size, "buf_bytes");

    // Direct call to vulnerable path
    entry_func(&c, buf, wrap, width, height, w, h, sides);
    return 0;
}
