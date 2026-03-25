/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifndef FZ_XML_MAX_DEPTH
#define FZ_XML_MAX_DEPTH 4096
#endif

// Minimal type definitions sufficient for the harness
typedef struct fz_context { int _dummy; } fz_context;
typedef struct fz_pool { int _dummy; } fz_pool;

typedef struct fz_xml fz_xml;
struct fz_xml {
    fz_xml *up;
    fz_xml *down;
    union {
        struct { fz_pool *pool; int refs; } doc;
        struct { fz_xml *next; } node; // present but unused
    } u;
};

typedef struct fz_buffer {
    unsigned char *data;
    size_t len;
} fz_buffer;

// Symbolic stub: allow KLEE to choose NULL or non-NULL to trigger the deref
