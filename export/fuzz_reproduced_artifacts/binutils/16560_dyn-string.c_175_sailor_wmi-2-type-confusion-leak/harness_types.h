/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Minimal local type model for dyn_string */
struct dyn_string {
    char *s;
    int length;
    int allocated;
};

typedef struct dyn_string* dyn_string_t;

/* Sets the contents of DS to the empty string.  */
void
