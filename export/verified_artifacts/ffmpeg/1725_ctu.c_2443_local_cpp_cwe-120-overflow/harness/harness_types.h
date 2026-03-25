/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifndef VVC_MAX_REF_ENTRIES
#define VVC_MAX_REF_ENTRIES 8
#endif

// Minimal project-specific structs to reach the vulnerable site
typedef struct CTU {
    int has_dmvr;
    int max_y[2][VVC_MAX_REF_ENTRIES];
} CTU;

typedef struct VVCFrameTab {
    CTU *ctus;
    const void **cus;  // unused in our slice
} VVCFrameTab;

typedef struct VVCFrameContext {
    VVCFrameTab tab;
} VVCFrameContext;

typedef struct H266RawSliceHeader {
    int num_ref_idx_active[2];
} H266RawSliceHeader;

typedef struct SliceHeaderWrap {
    H266RawSliceHeader *r;
} SliceHeaderWrap;

typedef struct SliceContext {
    SliceHeaderWrap sh;
} SliceContext;

typedef struct VVCLocalContext {
    VVCFrameContext *fc;
    SliceContext *sc;
} VVCLocalContext;

