/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

// Minimal type definitions needed by the harness
typedef struct Jbig2Image { int _dummy; } Jbig2Image;

typedef struct Jbig2SymbolDict {
    Jbig2Image **glyphs;
    uint32_t n_symbols;
} Jbig2SymbolDict;

typedef struct Jbig2Ctx {
    void *allocator; // placeholder; not used in this slice
} Jbig2Ctx;

// Simulate project allocator macro jbig2_new(ctx, Type, n) with 32-bit size overflow behavior
