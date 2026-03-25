/* AUTO-GENERATED from harness preamble */
#pragma once

/* minimal sliced harness for cavs.c:733 */
#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>

#ifndef NOT_AVAIL
#define NOT_AVAIL (-1)
#endif

/* Minimal AVSContext definition containing only what we touch here */
typedef struct AVSContext {
    int pred_mode_Y[3*3]; /* indices 0..8 */
} AVSContext;

/* Vulnerable function (neutralized) keeping ONLY the target statement */
