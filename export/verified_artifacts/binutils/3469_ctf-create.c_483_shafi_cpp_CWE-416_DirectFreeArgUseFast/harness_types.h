/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdint.h>
#include <stdlib.h>

// Minimal type definitions to compile the harness
typedef int ctf_id_t;
typedef struct ctf_dict ctf_dict_t; // opaque

typedef struct { uint32_t ctt_name; uint32_t ctt_info; uint32_t ctt_size; } ctf_type_t;
typedef struct ctf_dtdef {
    ctf_type_t dtd_data;
    void *dtd_vlen;
    ctf_id_t dtd_type;
} ctf_dtdef_t;

typedef struct { uint32_t cte_format; uint32_t cte_offset; uint32_t cte_bits; } ctf_encoding_t;

// ENTRY: direct pass-through — no guards, no checks
ctf_id_t ctf_add_encoded (ctf_dict_t *fp, uint32_t flag,
