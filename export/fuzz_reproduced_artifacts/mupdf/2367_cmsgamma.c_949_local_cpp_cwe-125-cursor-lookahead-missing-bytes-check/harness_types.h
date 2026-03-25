/* AUTO-GENERATED from harness preamble */
#pragma once

// harness/spine.c
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

// Minimal local definitions
#ifndef CMSEXPORT
#define CMSEXPORT
#endif

typedef void* cmsContext;
typedef uint32_t cmsUInt32Number;

#ifndef _cmsAssert
#define _cmsAssert(x) ((void)0)
#endif

// Forward decls
struct cmsToneCurve;

// Minimal segment type used in free path
typedef struct cmsCurveSegment_s {
    void *SampledPoints;
} cmsCurveSegment;

// Minimal tone curve structure with only fields used in free path
typedef struct cmsToneCurve {
    void *InterpParams;
    uint16_t *Table16;
    cmsCurveSegment *Segments;   // array of nSegments
    void **SegInterp;            // array of nSegments
    void *Evals;
    cmsUInt32Number nSegments;
} cmsToneCurve;

// Minimal internal free helpers (stubs calling libc free)

// Neutralized vulnerable callee kept intact on free-path
