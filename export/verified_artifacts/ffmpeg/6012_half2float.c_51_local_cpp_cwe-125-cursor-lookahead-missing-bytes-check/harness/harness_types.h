/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdint.h>
#include <stdlib.h>

// Minimal struct capturing the required tables
typedef struct Half2FloatTables {
    uint32_t mantissatable[4096];
    uint32_t exponenttable[64];
    uint32_t offsettable[64];
} Half2FloatTables;

// Entry function must be a simple pass-through (no guards)
