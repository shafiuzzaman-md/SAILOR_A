/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifndef PNG_IMAGE_ERROR
#define PNG_IMAGE_ERROR 1
#endif

/* Minimal structs to exercise the path */
struct png_control_def {
    void *error_buf;
};

typedef struct png_image {
    struct png_control_def *opaque;
    unsigned int warning_or_error;
    char message[256];
} png_image;

/* Decls for external helpers on the path */

/* ENTRY: must be a direct pass-through to the vulnerable function */
