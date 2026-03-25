/* AUTO-GENERATED from harness preamble */
#pragma once

// harness/spine.c
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

// Minimal local type definitions for harness
typedef struct { int dummy; } fz_context;

typedef struct fz_buffer_s {
    unsigned char *data;
    size_t cap;
    size_t len;
} fz_buffer;

// Stub for fz_realloc matching project API; use libc realloc
