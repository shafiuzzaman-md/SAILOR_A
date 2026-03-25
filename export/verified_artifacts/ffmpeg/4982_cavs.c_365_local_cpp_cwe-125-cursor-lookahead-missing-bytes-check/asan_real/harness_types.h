/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>

// Minimal local type mirroring only what's needed on the path
typedef struct AVSContext {
    int *pred_mode_Y;  // pointer so KLEE can track bounds
    int *top_pred_Y;   // unused in our sliced path
    int flags;         // unused in our sliced path
    int mbx;           // unused in our sliced path
} AVSContext;

// Vulnerable function — keep signature and the exact vulnerable statement
