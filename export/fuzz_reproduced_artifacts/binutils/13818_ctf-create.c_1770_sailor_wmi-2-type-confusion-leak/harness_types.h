/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// Minimal local definitions
#ifndef CTF_ADD_ROOT
#define CTF_ADD_ROOT 1u
#endif
#ifndef CTF_ERR
#define CTF_ERR (-1)
#endif

typedef int ctf_id_t;

typedef struct ctf_dictops {
    uint32_t isroot_mask;
} ctf_dictops_t;

typedef struct ctf_dict {
    ctf_dictops_t *ctf_dictops;
    struct ctf_dict *ctf_parent;
    void *ctf_add_processing;
    void *ctf_link_type_mapping;
} ctf_dict_t;

typedef struct ctf_type {
    uint32_t ctt_info;
} ctf_type_t;

typedef struct { int dummy; } ctf_bundle_t;  // placeholder, unused here

typedef struct { unsigned dummy; } ctf_encoding_t; // placeholder

// Macro that triggers the vulnerable dereference of fp->ctf_dictops
#ifndef LCTF_INFO_ISROOT
#define LCTF_INFO_ISROOT(fp, info) ((fp)->ctf_dictops->isroot_mask)
#endif

