/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdint.h>
#include <stddef.h>
#include <string.h>

// Minimal local LCMS-like types
#define CMSEXPORT

typedef void* cmsContext;
typedef unsigned char cmsUInt8Number;

typedef struct {
    cmsUInt8Number ID8[16];
} cmsProfileID;

typedef struct _cmsICCPROFILE {
    cmsProfileID ProfileID;
} _cmsICCPROFILE;

typedef _cmsICCPROFILE* cmsHPROFILE;

// Vulnerable function (neutralized) — keep exact vulnerable statement
