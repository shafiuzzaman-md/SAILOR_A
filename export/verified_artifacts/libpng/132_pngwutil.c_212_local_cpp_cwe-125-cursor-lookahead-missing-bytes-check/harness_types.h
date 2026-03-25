/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stddef.h>
#include <stdint.h>

// Minimal local typedefs/macros to compile the slice
#ifndef PNG_TYPES_DEFINED_LOCAL
#define PNG_TYPES_DEFINED_LOCAL 1
typedef unsigned char png_byte;
typedef unsigned int png_uint_32;
typedef struct { int dummy; } png_struct;  // opaque placeholder
#endif

#ifndef PNG_CHUNK_FROM_STRING
// Deliberately mirrors libpng semantics and indexes s[0..3]
#define PNG_CHUNK_FROM_STRING(s) \
    ((png_uint_32)((((png_uint_32)(s)[0]) << 24) | \
                   (((png_uint_32)(s)[1]) << 16) | \
                   (((png_uint_32)(s)[2]) << 8)  | \
                   ((png_uint_32)(s)[3])))
#endif

// External function (stubbed in stubs.c)
void png_write_complete_chunk(png_struct *png_ptr, png_uint_32 chunk_name,
                              const png_byte *data, size_t length);

// ENTRY = VULNERABLE FUNCTION (pass-through with exact vulnerable statement)
void png_write_chunk(png_struct *png_ptr, const png_byte *chunk_string,
