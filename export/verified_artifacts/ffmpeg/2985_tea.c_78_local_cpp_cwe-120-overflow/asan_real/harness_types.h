/* AUTO-GENERATED from harness preamble */
#pragma once

/* Minimal harness for tea.c:78 memcpy(iv, src, 8) overflow */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifndef KLEE_SAILOR_LOCAL_DEFS
#define KLEE_SAILOR_LOCAL_DEFS 1
#endif

// Minimal stand-in for FFmpeg's AVTEA
typedef struct AVTEA {
    uint32_t key[4];
    int rounds;
} AVTEA;

