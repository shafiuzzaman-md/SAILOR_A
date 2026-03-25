/* AUTO-GENERATED from harness preamble */
#pragma once

/* harness/spine.c */
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifndef MUJS_MAX_NAME
#define MUJS_MAX_NAME 64
#endif

/* Minimal opaque structures to satisfy signatures */
typedef struct js_Object {
    int dummy; /* minimal field to permit dereference */
} js_Object;

typedef struct js_State {
    js_Object *G; /* Global object pointer (target of UAF) */
} js_State;

/* Forward decl of internal function used by vulnerable line */

/* Vulnerable function (contains the exact vulnerable statement at jsrun.c:974) */
