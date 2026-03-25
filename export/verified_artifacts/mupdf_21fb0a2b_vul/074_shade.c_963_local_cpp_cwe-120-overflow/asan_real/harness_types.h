/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stddef.h>
#include <stdint.h>
#include <string.h>

// Minimal local type definitions to satisfy signatures
// We keep only the fields used by the harness/driver

typedef struct fz_context { int dummy; } fz_context;

typedef struct {
    int type;
    int use_function;
    void *colorspace;
    int ncomp; // drive number of components from driver
} fz_shade;

typedef struct { float a, b, c, d, e, f; } fz_matrix;

typedef struct { float x0, y0, x1, y1; } fz_rect;

typedef void (fz_shade_prepare_fn)(void *arg);

typedef void (fz_shade_process_fn)(void *arg, void *patch);

typedef struct fz_mesh_processor {
    int ncomp;
    fz_shade_process_fn *process;
    void *process_arg;
} fz_mesh_processor;

// Minimal patch structure holding color arrays (small destination to trigger overflow)
typedef struct {
    float color[4][4]; // destination buffers (size 4 floats each)
} fz_tensor_patch;

// Vulnerable function (neutralized to only the target operation)
