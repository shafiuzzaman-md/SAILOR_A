#include "harness_types.h"
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <klee/klee.h>

#ifndef PICT_FRAME
#define PICT_FRAME 3
#endif

extern int h264_entry(struct H264Context *h, struct H264SliceContext *sl);

int main() {
    struct H264Context *h = (struct H264Context *)calloc(1, sizeof(struct H264Context));
    struct H264SliceContext *sl = (struct H264SliceContext *)calloc(1, sizeof(struct H264SliceContext));
    if (!h || !sl) return 0;

    // Under-allocate H264Picture buffer to provoke OOB on memcpy(cur->ref_poc[1], ...)
    enum { PIC_SMALL = 128 }; // smaller than offset to ref_poc[1]
    void *raw = calloc(1, PIC_SMALL);
    if (!raw) return 0;
    // Make only the allocated bytes symbolic
    klee_make_symbolic(raw, PIC_SMALL, "pic_small_buf");

    // Cast under-sized buffer as H264Picture
    struct H264Picture *pic = (struct H264Picture *)raw;

    // Wire context
    h->cur_pic_ptr = pic;

    // Force target branch
    h->picture_structure = PICT_FRAME;

    // Call entry which directly calls the vulnerable function
    h264_entry(h, sl);

    return 0;
}
