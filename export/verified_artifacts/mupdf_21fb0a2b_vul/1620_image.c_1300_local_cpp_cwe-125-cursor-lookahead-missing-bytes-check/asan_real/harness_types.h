/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stddef.h>
#include <stdint.h>

// Minimal type declarations
typedef struct fz_context fz_context;
typedef struct fz_image fz_image;

typedef struct fz_buffer {
    size_t len;
    unsigned char *data;
} fz_buffer;

// Minimal image type identifiers
#ifndef FZ_IMAGE_UNKNOWN
#define FZ_IMAGE_UNKNOWN 0
#endif
#ifndef FZ_IMAGE_PNM
#define FZ_IMAGE_PNM 1
#endif
#ifndef FZ_IMAGE_JPX
#define FZ_IMAGE_JPX 2
#endif
#ifndef FZ_IMAGE_JPEG
#define FZ_IMAGE_JPEG 3
#endif
#ifndef FZ_IMAGE_PNG
#define FZ_IMAGE_PNG 4
#endif
#ifndef FZ_IMAGE_JXR
#define FZ_IMAGE_JXR 5
#endif
#ifndef FZ_IMAGE_TIFF
#define FZ_IMAGE_TIFF 6
#endif

// Vulnerable function — keep exact vulnerable statement, add sink assertion after
