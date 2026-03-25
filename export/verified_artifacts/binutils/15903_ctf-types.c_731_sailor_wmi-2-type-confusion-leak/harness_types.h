/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Minimal type defs
typedef uint32_t ctf_id_t;

typedef struct ctf_dict { int dummy; } ctf_dict_t;

typedef struct ctf_decl_node {
    int cd_kind;
    ctf_id_t cd_type;
} ctf_decl_node_t;

typedef struct ctf_decl { int cd_err; } ctf_decl_t;

typedef struct ctf_funcinfo { int ctc_argc; int ctc_flags; } ctf_funcinfo_t;

typedef int ctf_decl_prec_t; // not used in this slice

// Kinds (values arbitrary for harness)
#ifndef CTF_K_FUNCTION
#define CTF_K_FUNCTION 15
#endif

// Externals (smart-stubbed unless provided by stubs.c)

