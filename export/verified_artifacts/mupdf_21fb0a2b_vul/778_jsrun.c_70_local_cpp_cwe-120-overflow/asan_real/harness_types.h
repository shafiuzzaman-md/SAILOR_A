/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// Minimal type definitions sufficient for the vulnerable path

typedef struct js_String js_String;

typedef struct js_State {
    js_String *gcstr;
    int gccounter;
} js_State;

struct js_String {
    int gcmark;
    js_String *gcnext;
    char p[1]; // flexible tail (size computed via soffsetof)
};

#ifndef soffsetof
#define soffsetof(T, M) ((int)offsetof(T, M))
#endif

// Minimal allocator used by jsV_newmemstring
