#include "harness_types.h"
#include <klee/klee.h>
#include <stdint.h>
#include <stdlib.h>

// Globals referenced by harness/spine (mpegvideoencdsp.c)
uint8_t *g_buf;
ptrdiff_t g_wrap;
int g_width, g_height, g_w, g_h, g_sides;

// Entry function prototype from harness
void ff_mpegvideoencdsp_init(MpegvideoEncDSPContext *c, void *avctx);

int main() {
    // Allocate a concrete buffer larger than MAX_LINE_SIZE (1920)
    size_t buf_size = 4096;  // concrete, fixed
    g_buf = (uint8_t *)malloc(buf_size);
    if (!g_buf) return 0;
    klee_make_symbolic(g_buf, buf_size, "g_buf_bytes");

    // Make parameters symbolic to allow KLEE to explore OOB conditions
    klee_make_symbolic(&g_wrap, sizeof(g_wrap), "wrap");
    klee_make_symbolic(&g_width, sizeof(g_width), "width");
    klee_make_symbolic(&g_height, sizeof(g_height), "height");
    klee_make_symbolic(&g_w, sizeof(g_w), "w");
    klee_make_symbolic(&g_h, sizeof(g_h), "h");
    klee_make_symbolic(&g_sides, sizeof(g_sides), "sides");

    // Basic sanity to avoid undefined behavior while keeping OOB reachable
    klee_assume(g_wrap > 0);
    klee_assume(g_wrap <= 4096);
    klee_assume(g_width >= 0);
    klee_assume(g_width <= 4096);
    klee_assume(g_height >= 0);
    klee_assume(g_height <= 4096);
    klee_assume(g_w >= 0);
    klee_assume(g_w <= 4096);
    klee_assume(g_h >= 0);
    klee_assume(g_h <= 4096);
    // g_sides unconstrained; not used in our spine

    // Call the entry function (directly triggers the vulnerable write inside)
    ff_mpegvideoencdsp_init(NULL, NULL);
    return 0;
}
