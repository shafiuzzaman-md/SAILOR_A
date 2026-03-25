/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdlib.h>
#include <string.h>

// Minimal local definitions to satisfy compilation
typedef int cmsBool;
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

typedef void* cmsContext;

typedef struct cmsPluginBase_s { int dummy; } cmsPluginBase;

typedef void (*IntentLinkFn)(void); // placeholder, not invoked in this slice

typedef struct {
    int Intent;
    const char *Description;   // source string (may be non-NUL terminated)
    IntentLinkFn Link;
} cmsPluginRenderingIntent;

typedef struct cmsIntentsList_s {
    int Intent;
    char Description[64];      // destination buffer size (controls strncpy count)
    IntentLinkFn Link;
    struct cmsIntentsList_s *Next;
} cmsIntentsList;

typedef struct {
    cmsIntentsList *Intents;
} _cmsIntentsPluginChunkType;

// Stubs (simple, over-approximating)

