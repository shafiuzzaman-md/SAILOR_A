/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Minimal type definitions needed for the path */
typedef struct js_Object {
    int tag; /* minimal field so stubs can dereference */
} js_Object;

typedef struct js_State {
    js_Object *R; /* registry object pointer (target of lifecycle mismatch) */
    int strict;   /* keep minimal extra field to resemble real type */
} js_State;

/* Prototype for helper used at the vulnerable site */

/* Entry function: neutralized pass-through to vulnerable function */
