/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdlib.h>
#include <stdint.h>

/* Minimal local types to satisfy signatures and field accesses */
typedef struct png_struct_def {
    unsigned int mode;
    unsigned int mng_features_permitted;
    int unknown_default;
} png_struct;

typedef struct png_unknown_chunk_def {
    char name[5];
    unsigned int size;
    char *data;
    unsigned int location;
} png_unknown_chunk;

typedef struct png_info_def {
    /* Only the fields referenced by the harness are needed */
    unsigned int width, height;
    unsigned char bit_depth, color_type, compression_type, filter_type, interlace_type;
    unsigned int valid;
    unsigned int num_frames, num_plays;
    const png_unknown_chunk *unknown_chunks;
    unsigned int unknown_chunks_num;
} png_info;

/* Forward decl of vulnerable function */

