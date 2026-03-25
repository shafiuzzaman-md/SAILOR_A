/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdint.h>
#include <stddef.h>

/* Minimal typedefs matching libpng widths */
typedef int32_t  png_int_32;
typedef uint32_t png_uint_32;
typedef uint8_t  png_byte;

/* Local stand-ins for libpng types used by this function */
typedef struct png_struct_def { int dummy; } png_struct;
typedef struct png_info_def {
    png_uint_32 valid;
    png_byte    offset_unit_type;
    png_int_32  x_offset;
} png_info;

/* Define the macros used in the target function */
#ifndef PNG_oFFs_SUPPORTED
#define PNG_oFFs_SUPPORTED
#endif
#ifndef PNG_INFO_oFFs
#define PNG_INFO_oFFs 0x1u
#endif
#ifndef PNG_OFFSET_PIXEL
#define PNG_OFFSET_PIXEL 0
#endif
