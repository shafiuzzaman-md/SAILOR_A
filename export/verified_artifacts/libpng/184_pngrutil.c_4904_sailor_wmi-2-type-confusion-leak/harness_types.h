/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdint.h>
#include <stdlib.h>

// Minimal local typedefs to satisfy signatures
typedef uint32_t png_uint_32;

typedef struct png_struct_def {
    png_uint_32 width;  // used by vulnerable path
} png_struct;

typedef struct png_info_def {
    png_uint_32 next_frame_width; // vulnerable read source
} png_info;

// Forward decl of vulnerable function

