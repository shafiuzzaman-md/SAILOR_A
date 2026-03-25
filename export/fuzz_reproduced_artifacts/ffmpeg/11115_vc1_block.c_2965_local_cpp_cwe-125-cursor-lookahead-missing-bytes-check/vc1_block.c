#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <klee/klee.h>

// Minimal types to support the sliced path

typedef struct {
    uint8_t *data[4];
} PictureLike;

typedef struct {
    // Fields used by the vulnerable statements
    PictureLike last_pic;
    uint8_t *dest[4];
    int linesize;
    int uvlinesize;
    int mb_y;
} MpegEncContext;

typedef struct VC1Context {
    MpegEncContext s;
} VC1Context;

// Sliced vulnerable function containing the real vulnerable statements
static void vc1_decode_skip_blocks(VC1Context *v)
{
    MpegEncContext *s = &v->s;

    if (!v->s.last_pic.data[0])
        return;

    // Vulnerable statements from vc1_block.c (verbatim):
    memcpy(s->dest[0], s->last_pic.data[0] + s->mb_y * 16 * s->linesize,   s->linesize   * 16);
    klee_assert(0 && "SAILOR_SINK_REACHED");
    memcpy(s->dest[1], s->last_pic.data[1] + s->mb_y *  8 * s->uvlinesize, s->uvlinesize *  8);
    memcpy(s->dest[2], s->last_pic.data[2] + s->mb_y *  8 * s->uvlinesize, s->uvlinesize *  8);
}

// Entry function: direct pass-through with no guards
void ff_vc1_decode_blocks(VC1Context *v)
{
    vc1_decode_skip_blocks(v);
}
