/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdint.h>
#include <stdlib.h>

// Minimal local LCMS-like typedefs to compile standalone
typedef void* cmsContext;
typedef float cmsFloat32Number;
typedef double cmsFloat64Number;
typedef int32_t cmsInt32Number;
typedef uint32_t cmsUInt32Number;

#ifndef PLUS_INF
#define PLUS_INF (1.0/0.0)
#endif

// Minimal structures used by the function
typedef struct {
    cmsFloat64Number x0;
    cmsFloat64Number x1;
    int Type;
    cmsUInt32Number nGridPoints;
    cmsFloat32Number* SampledPoints;
    cmsFloat32Number Params[5];
} cmsCurveSegment;

typedef struct { int dummy; } cmsToneCurve;

