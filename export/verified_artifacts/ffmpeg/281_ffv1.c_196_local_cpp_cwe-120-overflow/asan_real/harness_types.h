/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifndef MAX_QUANT_TABLES
#define MAX_QUANT_TABLES 8
#endif
#ifndef CONTEXT_SIZE
#define CONTEXT_SIZE 32
#endif
#ifndef AC_GOLOMB_RICE
#define AC_GOLOMB_RICE 0
#endif

// Minimal structs for the vulnerable path
typedef struct PlaneContext {
    int quant_table_index;
    int context_count;
    uint8_t *state;
    struct { int drift, error_sum, bias, count; } *vlc_state; // neutralized
} PlaneContext;

typedef struct FFV1SliceContext {
    PlaneContext *plane; // array of plane_count elements
} FFV1SliceContext;

typedef struct FFV1Context {
    int plane_count;
    int ac;                              // 1=range coder <-> 0=golomb rice
    uint8_t *initial_states[MAX_QUANT_TABLES];
} FFV1Context;

