/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifndef _
#define _(s) (s)
#endif

// Minimal local type definitions sufficient for the harness path
typedef int ctf_id_t;
typedef int ctf_sect_names_t;
typedef struct ctf_next { int dummy; } ctf_next_t;
typedef void ctf_dump_decorate_f;

typedef struct {
    void *cts_data;
} ctf_symtab_ext_t;

typedef struct ctf_dict_s {
    void *ctf_funcidx_names;
    void *ctf_objtidx_names;
    ctf_symtab_ext_t ctf_ext_symtab;
} ctf_dict_t;

typedef struct ctf_dump_state {
    int dummy;
} ctf_dump_state_t;

// Forward declaration of the vulnerable function

// ENTRY FUNCTION — neutralized pass-through (MANDATORY pattern)
