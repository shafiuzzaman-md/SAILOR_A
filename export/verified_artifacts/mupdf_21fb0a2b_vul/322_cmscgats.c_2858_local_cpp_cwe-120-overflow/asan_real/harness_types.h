/* AUTO-GENERATED from harness preamble */
#pragma once


// spine.c — minimal sliced harness for cmsIT8GetPatchName overflow
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// Minimal local definitions
#ifndef CMSEXPORT
#define CMSEXPORT
#endif

typedef void* cmsContext;
typedef void* cmsHANDLE;

#ifndef MAXID
#define MAXID 128
#endif
#ifndef MAXSTR
#define MAXSTR 1024
#endif
#ifndef MAXTABLES
#define MAXTABLES 255
#endif
#ifndef MAXINCLUDE
#define MAXINCLUDE 20
#endif

// Minimal structs needed by the vulnerable function path
typedef struct TABLE { int SampleID; } TABLE;
typedef struct cmsIT8 { char *user_data; } cmsIT8;

// Stubs used by the vulnerable function
static TABLE g_table = { .SampleID = 0 };
