/* AUTO-GENERATED from harness preamble */
#pragma once

/* minimal harness spine for png_write_info -> png_write_info_before_PLTE */
#include <stddef.h>
#include <stdint.h>

/* Minimal typedefs/macros to compile the vulnerable line verbatim */
typedef uint32_t png_uint_32;
typedef unsigned char png_byte;

typedef struct png_struct_def {
    unsigned int transformations;
    unsigned int mode;
    unsigned int unknown_default;
} png_struct;

typedef struct png_info_def {
    png_uint_32 valid;
    png_uint_32 num_frames;
    png_uint_32 num_plays;
    /* other fields omitted */
} png_info;

#ifndef PNG_INFO_acTL
#define PNG_INFO_acTL 0x00000001u
#endif

/* Stub prototype used in vulnerable function */

