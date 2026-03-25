/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdlib.h>
#include <stdint.h>

// Minimal type forward declarations
typedef struct fz_context fz_context;
typedef struct pdf_obj pdf_obj;

// Minimal object model needed by DICT/ITEMS access
struct pdf_obj {
    short refs;
    unsigned char kind;
    unsigned char flags;
};

struct keyval {
    pdf_obj *k;
    pdf_obj *v;
};

typedef struct pdf_obj_dict {
    struct pdf_obj super;
    int len;
    struct keyval *items;
} pdf_obj_dict;

#define DICT(o) ((pdf_obj_dict *)(o))

// Provide a concrete PDF_NULL we can assign to
#define PDF_NULL (&pdf_null_obj)

