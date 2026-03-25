#include <stdint.h>
#include <stdlib.h>
#include <klee/klee.h>

// Minimal macro definitions to drive the vulnerable path
#ifndef FRAME_MBAFF
#define FRAME_MBAFF(h) 1
#endif
#ifndef IS_INTERLACED
#define IS_INTERLACED(x) ((x) & 1)
#endif
#ifndef LTOP
#define LTOP 0
#endif

// Minimal types needed for the vulnerable access
typedef struct {
    int *mb_type; // minimal: only mb_type[] is used for mb_type
} H264Picture;

typedef struct H264Context {
    int mb_stride;
    H264Picture cur_pic;
    int flags; // unused but common in code
} H264Context;

typedef struct H264SliceContext {
    const struct H264Context *h264; // placeholder, not used directly here
    int *left_type; // pointer so we can create 0-sized allocation to trigger OOB
    int slice_alpha_c0_offset; // unused here
    int slice_beta_offset;     // unused here
} H264SliceContext;

// Vulnerable function (neutralized to the target condition only)
void ff_h264_filter_mb(const H264Context *h, H264SliceContext *sl,
                       int mb_x, int mb_y,
                       uint8_t *img_y, uint8_t *img_cb, uint8_t *img_cr,
                       unsigned int linesize, unsigned int uvlinesize)
{
    const int mb_xy = mb_x + mb_y * h->mb_stride;
    const int mb_type = h->cur_pic.mb_type[mb_xy];

    if (FRAME_MBAFF(h)
            // and current and left pair do not have the same interlaced type
            && IS_INTERLACED(mb_type ^ sl->left_type[LTOP])
            // and left mb is in available to us
            && sl->left_type[LTOP]) {
        // If we reach here without crashing on the lookahead reads above,
        // mark reachability. KLEE will natively flag OOB/null on the conditions.
        klee_assert(0 && "SAILOR_SINK_REACHED");
    }
}

// Mandatory pass-through entry function (no guards!)
int entry_func(const H264Context *h, H264SliceContext *sl,
               int mb_x, int mb_y,
               uint8_t *img_y, uint8_t *img_cb, uint8_t *img_cr,
               unsigned int linesize, unsigned int uvlinesize) {
    ff_h264_filter_mb(h, sl, mb_x, mb_y, img_y, img_cb, img_cr, linesize, uvlinesize);
    return 0;
}
