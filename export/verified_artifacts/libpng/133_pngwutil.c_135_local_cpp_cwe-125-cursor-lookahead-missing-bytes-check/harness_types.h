/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stddef.h>
#include <stdint.h>

/* Minimal local typedefs/macros to compile harness */
typedef unsigned char png_byte;
typedef unsigned int png_uint_32;
typedef struct png_struct_def png_struct; /* opaque; driver will pass NULL */

#ifndef PNG_CHUNK_FROM_STRING
#define PNG_CHUNK_FROM_STRING(s) \
    (((png_uint_32)(s)[0] << 24) + ((png_uint_32)(s)[1] << 16) + \
     ((png_uint_32)(s)[2] << 8) + (png_uint_32)(s)[3])
#endif

/* Neutralized stub: no side effects; vuln is in argument evaluation */
