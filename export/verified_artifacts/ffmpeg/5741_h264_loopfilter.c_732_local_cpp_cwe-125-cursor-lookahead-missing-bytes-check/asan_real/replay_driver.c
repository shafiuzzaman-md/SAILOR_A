// Combined reproducer for 5741_h264_loopfilter.c_732_local_cpp_cwe-125-cursor-lookahead-missing-bytes-check
// Original harness: driver.c + smart_stubs.c + sliced source

// === smart_stubs.c ===
/* Smart stubs — auto-generated from path + vulnerability analysis */
/* Symbolic stubs model the environment: KLEE explores return values */
/* that both REACH the sink AND TRIGGER the vulnerability */
#include <stdlib.h>
#include <string.h>
#include <klee/klee.h>

/* PROACTIVE: function (auto-detected external) */
int function() { return 0; }

// === driver.c ===
// NO_HARNESS_TYPES
#include <stdint.h>
#include <stdlib.h>
#include <klee/klee.h>

// Replicate minimal types used in harness/h264_loopfilter.c
typedef struct {
    int *mb_type;
} H264Picture;

typedef struct H264Context {
    int mb_stride;
    H264Picture cur_pic;
    int flags;
} H264Context;

typedef struct H264SliceContext {
    const struct H264Context *h264;
    int *left_type;
    int slice_alpha_c0_offset;
    int slice_beta_offset;
} H264SliceContext;

// Prototype of entry_func as defined in harness
int entry_func(const H264Context *h, H264SliceContext *sl,
               int mb_x, int mb_y,
               uint8_t *img_y, uint8_t *img_cb, uint8_t *img_cr,
               unsigned int linesize, unsigned int uvlinesize);

int main() {
    // Allocate contexts
    H264Context *h = (H264Context *)calloc(1, sizeof(H264Context));
    H264SliceContext *sl = (H264SliceContext *)calloc(1, sizeof(H264SliceContext));

    // Set minimal mb geometry so mb_xy == 0
    h->mb_stride = 1;

    // Allocate one mb_type entry and make it symbolic
    int *mb_type_arr = (int *)malloc(sizeof(int));
    klee_make_symbolic(mb_type_arr, sizeof(int), "mb_type0");
    h->cur_pic.mb_type = mb_type_arr;

    // Create a too-small buffer for left_type to force OOB on left_type[0]
    char *tiny = (char *)malloc(1);
    klee_make_symbolic(tiny, 1, "left_type_tiny");
    sl->left_type = (int *)tiny; // misaligned, size=1 -> OOB on int access

    // Dummy frame buffers (not used on the path to the sink)
    uint8_t *img_y  = (uint8_t *)malloc(16);
    uint8_t *img_cb = (uint8_t *)malloc(16);
    uint8_t *img_cr = (uint8_t *)malloc(16);
    klee_make_symbolic(img_y, 16, "img_y");
    klee_make_symbolic(img_cb, 16, "img_cb");
    klee_make_symbolic(img_cr, 16, "img_cr");

    unsigned int linesize = 4, uvlinesize = 4;

    // Call entry (no guards in harness)
    entry_func(h, sl, 0, 0, img_y, img_cb, img_cr, linesize, uvlinesize);
    return 0;
}
