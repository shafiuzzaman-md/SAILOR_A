/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdint.h>
#include <stdlib.h>

// Minimal stand-ins for MuPDF types used in signature
typedef struct { int throw_on_repair; } fz_context;

typedef struct {
    int is_fdf;
    int repair_attempted;
    struct { int dummy; } lexbuf; // placeholder; not used in harness body
} pdf_document;

// Local copy of the struct used in the repair routine
struct entry {
    int num;
    int gen;
    int64_t ofs;
    int64_t stm_ofs;
    int64_t stm_len;
};

typedef struct {
    char type;
    int64_t ofs;
    int gen;
    int num;
    int64_t stm_ofs;
} pdf_xref_entry;

// Stub: pdf_get_populating_xref_entry returns a stable entry object
