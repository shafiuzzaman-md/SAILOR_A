/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

// Minimal type definitions for the harness
typedef struct ctf_dict_t { int dummy; } ctf_dict_t;

typedef struct ctf_dump_state_t { int dummy; } ctf_dump_state_t;

typedef int ctf_sect_names_t;           // enum in real code, int is fine here

typedef void ctf_dump_decorate_f;       // incomplete type; pointer is fine

// Entry function: MUST be a simple pass-through (no guards)
char *
ctf_dump (ctf_dict_t *fp, ctf_dump_state_t **statep, ctf_sect_names_t sect,
