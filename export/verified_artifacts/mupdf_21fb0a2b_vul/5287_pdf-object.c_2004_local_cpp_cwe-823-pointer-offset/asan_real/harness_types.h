/* AUTO-GENERATED from harness preamble */
#pragma once

// harness/spine.c
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>

// Minimal local types/macros to support the sliced function

typedef struct fz_context { int dummy; } fz_context;

typedef struct pdf_obj {
    short refs;
    unsigned char kind;
    unsigned char flags;
} pdf_obj;

typedef struct pdf_array_s {
    pdf_obj super;
    int len;
    int cap;
    pdf_obj **items;
} pdf_array;

// Kinds (match MuPDF conventions)
typedef enum pdf_objkind_e {
    PDF_INT = 'i',
    PDF_REAL = 'f',
    PDF_STRING = 's',
    PDF_NAME = 'n',
    PDF_ARRAY = 'a',
    PDF_DICT = 'd',
    PDF_INDIRECT = 'r'
} pdf_objkind;

// Macros from MuPDF simplified
#define ARRAY(o) ((pdf_array*)(o))
