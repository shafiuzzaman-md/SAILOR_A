/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stddef.h>
#include <stdint.h>

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

/* Minimal JPEG types needed for the vulnerable statements */
typedef struct jpeg_component_info {
    int component_needed;
} jpeg_component_info;

typedef struct jpeg_decompress_struct {
    int out_color_space;
    int out_color_components;
    int num_components;
    int jpeg_color_space;
    int color_transform;
    struct jpeg_component_info *comp_info; /* array */
} jpeg_decompress_struct;

typedef jpeg_decompress_struct * j_decompress_ptr;

/* Color space constants (minimal subset) */
#ifndef JCS_YCCK
#define JCS_YCCK 7
#endif

/* Neutralized vulnerable function: keep only the target switch-case and the
 * exact vulnerable statements. */
