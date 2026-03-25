/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdint.h>

// Minimal type definitions to satisfy signatures and field access in the vuln line
typedef unsigned int ctf_id_t;

typedef struct ctf_dynhash ctf_dynhash_t;  // opaque

typedef struct ctf_dict {
    ctf_dynhash_t *ctf_funchash;
    ctf_dynhash_t *ctf_objthash;
} ctf_dict_t;

typedef struct ctf_next ctf_next_t;  // opaque

