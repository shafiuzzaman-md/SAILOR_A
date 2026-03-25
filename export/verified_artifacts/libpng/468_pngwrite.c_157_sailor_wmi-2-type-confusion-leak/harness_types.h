/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdint.h>
#include <stdlib.h>

// Minimal local type/macro defs sufficient for the harness
#ifndef PNG_INFO_sBIT
#define PNG_INFO_sBIT 0x0008u
#endif

typedef uint32_t png_uint_32;
typedef struct png_struct_def { int dummy; } png_struct;
typedef struct png_info_def {
    png_uint_32 valid;
    int color_type;
} png_info;

// Decls

