/* AUTO-GENERATED from harness preamble */
#pragma once

/* Minimal sliced harness for simple-object.c vulnerability */
#include <stdint.h>
#include <stdlib.h>

/* Minimal type definitions needed by the vulnerable statement */
typedef struct simple_object_functions {
    void (*release_write)(void *data);
} simple_object_functions;

typedef struct simple_object_write {
    simple_object_functions *functions;
    void *data;
} simple_object_write;

/* The entry uses a read-type pointer, which we forward-declare minimally. */
typedef struct simple_object_read {
    /* opaque; not used by harness */
    int _opaque;
} simple_object_read;

