/* AUTO-GENERATED from harness preamble */
#pragma once

/* auto-generated minimal harness for vc1_mc.c:847 */
#include <stdint.h>
#include <stdlib.h>

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
