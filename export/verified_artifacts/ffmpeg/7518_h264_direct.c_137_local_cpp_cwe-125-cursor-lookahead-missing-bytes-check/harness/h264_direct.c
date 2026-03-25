#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <klee/klee.h>

#ifndef PICT_FRAME
#define PICT_FRAME 3
#endif

// Minimal types to support the path
typedef struct H264Picture {
    int ref_count[2][2];
    int ref_poc[2][2][32];
    int mbaff;
    int poc;
} H264Picture;

typedef struct H264Context {
    int picture_structure;
    H264Picture *cur_pic_ptr;
} H264Context;

typedef struct H264SliceContext { int dummy; } H264SliceContext;

// Vulnerable function (neutralized to the minimal path containing the sink)
void ff_h264_direct_ref_list_init(const H264Context *const h, H264SliceContext *sl)
{
    H264Picture *const cur = h->cur_pic_ptr;

    if (h->picture_structure == PICT_FRAME) {
        memcpy(cur->ref_count[1], cur->ref_count[0], sizeof(cur->ref_count[0]));
        memcpy(cur->ref_poc[1],   cur->ref_poc[0],   sizeof(cur->ref_poc[0]));
        // Universal sink assertion placed AFTER the vulnerable statement
        klee_assert(0 && "SAILOR_SINK_REACHED");
    }
}

// Entry function MUST be a direct pass-through
int h264_entry(H264Context *h, H264SliceContext *sl) {
    ff_h264_direct_ref_list_init(h, sl);
    return 0;
}
