/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifndef CMSEXPORT
#define CMSEXPORT
#endif
#ifndef cmsUNUSED_PARAMETER
#define cmsUNUSED_PARAMETER(x) (void)(x)
#endif

typedef void* cmsContext;
typedef void* cmsHPROFILE;
typedef unsigned char cmsUInt8Number;

typedef struct {
    struct { cmsUInt8Number ID8[16]; } ProfileID;
} _cmsICCPROFILE;

// Vulnerable function (from cmsio0.c around line 1052)
