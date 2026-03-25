/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

// Minimal local definitions to satisfy signatures
#ifndef HMVP_MAX
#define HMVP_MAX 8
#endif

// Simplified MV/MVField types
typedef struct { int16_t x, y; } MvField;

typedef struct EntryPoint {
    MvField hmvp[HMVP_MAX];
    int num_hmvp;
    MvField hmvp_ibc[HMVP_MAX];
    int num_hmvp_ibc;
} EntryPoint;

typedef struct { int min_pu_width; } PPS;

typedef struct {
    struct { PPS *pps; } ps;
    struct { MvField *mvf; } tab;
} VVCFrameContext;

typedef struct {
    int pred_mode;
    int cb_width, cb_height;
    int x0, y0;
} CodingUnit;

typedef struct VVCLocalContext {
    const VVCFrameContext *fc;
    const CodingUnit *cu;
    EntryPoint *ep;
} VVCLocalContext;

typedef struct MotionInfo { int dummy; } MotionInfo;

typedef int (*compare_fn)(const MvField*, const MvField*);

// Dummy compare callbacks
static int compare_l0_mv(const MvField *a, const MvField *b) { (void)a; (void)b; int r=0; klee_make_symbolic(&r, sizeof(r), "cmp_l0"); return r; }
static int compare_mv_ref_idx(const MvField *a, const MvField *b) { (void)a; (void)b; int r=0; klee_make_symbolic(&r, sizeof(r), "cmp_ref"); return r; }

// Sliced vulnerable helper: keep the memmove sink
