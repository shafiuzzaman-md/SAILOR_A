/* AUTO-GENERATED from harness preamble */
#pragma once


#include <stdint.h>
#include <stddef.h>
#include <string.h>

// Minimal local typedefs
typedef unsigned char byte;

typedef struct { int dummy; } Jbig2Ctx;

typedef struct {
    unsigned int number;
    size_t data_length;
} Jbig2Segment;

typedef struct Jbig2SymbolDict Jbig2SymbolDict; // opaque

typedef struct {
    int SDTEMPLATE;           // controls sdat_bytes
    const byte *sdat;         // source data for memcpy
} Jbig2SymbolDictParams;

typedef struct { int dummy; } Jbig2ArithCx;

// Global params so driver can configure them before call
Jbig2SymbolDictParams g_params;

// Forward declaration of vulnerable function
static Jbig2SymbolDict * jbig2_decode_symbol_dict(Jbig2Ctx *ctx, Jbig2Segment *segment,
    const Jbig2SymbolDictParams *params, const byte *data, size_t size,
    Jbig2ArithCx *GB_stats, Jbig2ArithCx *GR_stats);

