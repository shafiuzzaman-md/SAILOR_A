/* auto-generated minimal harness for vc1_mc.c:847 */
#include <stdint.h>
#include <stdlib.h>
#include <klee/klee.h>

// Minimal stand-ins for project types: only fields we touch
typedef struct MpegEncContext {
    int *block_index;   // used at sink: s->block_index[0]
    int v_edge_pos;     // declared in real code, not used here
} MpegEncContext;

typedef struct H264ChromaContext {
    int dummy;
} H264ChromaContext;

typedef struct VC1Context {
    MpegEncContext s;           // accessed as &v->s
    H264ChromaContext h264chroma; // present in signature context, unused here
    int *blk_mv_type;           // indexed by s->block_index[0]
} VC1Context;

// VULNERABLE FUNCTION (neutralized, keep only the target statement)
// Signature must match original
void ff_vc1_mc_4mv_chroma4(VC1Context *v, int dir, int dir2, int avg) {
    MpegEncContext *s = &v->s;
    // Vulnerable statement copied verbatim from source (vc1_mc.c:847):
    int fieldmv = v->blk_mv_type[s->block_index[0]];
    (void)fieldmv; // silence unused var in harness
    // Universal sink assertion — only fires if statement didn't crash
    klee_assert(0 && "SAILOR_SINK_REACHED");
}

// ENTRY: must be a pure pass-through (no guards)
int entry_func(VC1Context *v, int dir, int dir2, int avg) {
    ff_vc1_mc_4mv_chroma4(v, dir, dir2, avg);
    return 0;
}
