/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

// Minimal local definitions (opaque enough for harness)
typedef struct js_State { int dummy; } js_State;

typedef struct js_Property {
    struct js_Property *left;
    struct js_Property *right;
    const char *name;
    int atts;
} js_Property;

typedef struct js_Object {
    js_Property *properties;
} js_Object;

// Neutral stub of deleteproperty reflecting signature from source context.
// Keep it simple: return the incoming prop without dereferencing (so the sink assertion can fire if no crash).
