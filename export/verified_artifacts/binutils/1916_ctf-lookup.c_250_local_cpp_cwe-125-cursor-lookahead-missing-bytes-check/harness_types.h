/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Minimal type defs to compile harness
typedef unsigned long ctf_id_t;

typedef struct ctf_lookup {
  const char *ctl_prefix;
  size_t ctl_len;
} ctf_lookup_t;

typedef struct ctf_dict {
  const ctf_lookup_t *ctf_lookups;
  size_t ctf_tmp_typeslicelen;
  char *ctf_tmp_typeslice;
} ctf_dict_t;

// ENTRY: direct pass-through (no guards!)
