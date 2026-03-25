/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdint.h>
#include <stddef.h>
#include <string.h>

#ifndef STATE_SIZE
#define STATE_SIZE 16
#endif

typedef struct AVMurMur3 {
    uint64_t h1;
    uint64_t h2;
    uint8_t  state[STATE_SIZE];
    int      state_pos;
    size_t   len;
} AVMurMur3;

