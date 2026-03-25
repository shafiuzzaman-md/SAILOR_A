/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#ifndef JBIG2_SEVERITY_FATAL
#define JBIG2_SEVERITY_FATAL 2
#endif
#ifndef JBIG2_SEVERITY_WARNING
#define JBIG2_SEVERITY_WARNING 1
#endif
#ifndef JBIG2_UNKNOWN_SEGMENT_NUMBER
#define JBIG2_UNKNOWN_SEGMENT_NUMBER 0
#endif

typedef struct _Jbig2Allocator { int dummy; } Jbig2Allocator;

typedef struct _Jbig2Ctx {
    Jbig2Allocator *allocator;
    int err;
} Jbig2Ctx;

typedef struct _Jbig2Image {
    uint32_t width;
    uint32_t height;
    uint32_t stride;
    uint8_t *data;
} Jbig2Image;

