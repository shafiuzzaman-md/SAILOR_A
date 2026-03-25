/* AUTO-GENERATED from harness preamble */
#pragma once

/* minimal harness slice for pdf_update_object -> pdf_delete_object (revised) */
#include <stdint.h>
#include <stddef.h>

/* Minimal type definitions to match fields used in the slice */
typedef struct pdf_xref_entry {
    int type;
    int gen;
    int num;
    int stm_ofs;
    void *stm_buf;
    void *obj;
} pdf_xref_entry;

typedef struct pdf_xref_subsec {
    int start;
    int len;
    struct pdf_xref_subsec *next;
    pdf_xref_entry *table;
} pdf_xref_subsec;

typedef struct pdf_xref {
    int num_objects;
    pdf_xref_subsec *subsec;
} pdf_xref;

typedef struct pdf_document {
    int local_xref;
    int local_xref_nesting;
    int num_xref_sections;
    pdf_xref *xref_sections; /* points to an array of pdf_xref */
} pdf_document;

typedef struct fz_context { int dummy; } fz_context;

typedef struct pdf_obj { int dummy; } pdf_obj;

/* Forward decl */

/* ENTRY: Neutralized pass-through: DIRECT call to vulnerable function, no guards */
