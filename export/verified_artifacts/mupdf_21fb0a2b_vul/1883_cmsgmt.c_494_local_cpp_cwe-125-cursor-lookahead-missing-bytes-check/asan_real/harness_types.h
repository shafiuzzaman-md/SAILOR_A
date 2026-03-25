/* AUTO-GENERATED from harness preamble */
#pragma once

/* Minimal sliced harness for cmsDetectTAC reaching cmsgmt.c:494 */
#include <stddef.h>
#include <stdint.h>

// Local type/macro shims (minimal)
typedef void* cmsContext;
typedef void* cmsHPROFILE;
typedef void* cmsHTRANSFORM;
typedef uint32_t cmsUInt32Number;
typedef double cmsFloat64Number;
typedef float cmsFloat32Number;

#ifndef cmsMAXCHANNELS
#define cmsMAXCHANNELS 32
#endif
#ifndef MAX_INPUT_DIMENSIONS
#define MAX_INPUT_DIMENSIONS 16
#endif

// From cmsgmt.c preamble (sliced)
typedef struct {
    cmsUInt32Number  nOutputChans;
    cmsHTRANSFORM    hRoundTrip;
    cmsFloat32Number MaxTAC;
    cmsFloat32Number MaxInput[cmsMAXCHANNELS];
} cmsTACestimator;

// Vulnerable function (neutralized) — keep the exact sink line
