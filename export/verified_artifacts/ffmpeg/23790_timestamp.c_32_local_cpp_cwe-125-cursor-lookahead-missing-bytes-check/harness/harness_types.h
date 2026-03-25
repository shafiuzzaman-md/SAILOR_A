/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdint.h>
#include <stdio.h>
#include <math.h>

#ifndef AV_TS_MAX_STRING_SIZE
#define AV_TS_MAX_STRING_SIZE 1  // force edge case to make last become -1
#endif

#ifndef AV_NOPTS_VALUE
#define AV_NOPTS_VALUE (-9223372036854775807LL - 1)
#endif

#ifndef FFMIN
#define FFMIN(a,b) ((a) > (b) ? (b) : (a))
#endif

#ifndef INFINITY
#define INFINITY (1.0/0.0)
#endif

// Minimal AVRational definition (FFmpeg-compatible signature)
typedef struct AVRational { int num; int den; } AVRational;
