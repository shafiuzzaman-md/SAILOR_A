/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

/* Minimal type shims to compile harness */
typedef unsigned long ctf_id_t;

typedef struct ctf_dict_s {
    uint32_t *ctf_pptrtab;
    size_t ctf_pptrtab_len;
    /* Driver-controlled new length for neutralized pass-through */
    size_t driver_new_len;
} ctf_dict_t;

