/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

// Minimal local typedefs/macros to compile the sliced function
typedef unsigned int JDIMENSION;
typedef unsigned char JSAMPLE;
typedef JSAMPLE* JSAMPROW;
typedef JSAMPROW* JSAMPARRAY;

typedef short JCOEF;
typedef JCOEF* JCOEFPTR;

typedef int DCTELEM;
typedef int ISLOW_MULT_TYPE;

typedef struct jpeg_decompress_struct {
    JSAMPLE *sample_range_limit;
} * j_decompress_ptr;

typedef struct jpeg_component_info {
    void *dct_table;
} jpeg_component_info;

#ifndef RANGE_CENTER
#define RANGE_CENTER 128
#endif
#ifndef RANGE_MASK
#define RANGE_MASK 2047
#endif
#define ISHIFT_TEMPS
#define IRIGHT_SHIFT(x,shft) ((x) >> (shft))
