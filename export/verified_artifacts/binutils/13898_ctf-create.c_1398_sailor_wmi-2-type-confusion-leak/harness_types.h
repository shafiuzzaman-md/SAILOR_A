/* AUTO-GENERATED from harness preamble */
#pragma once

/* Minimal sliced harness for ctf_add_funcobjt_sym -> ctf_add_funcobjt_sym_forced */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifndef CTF_K_FUNCTION
#define CTF_K_FUNCTION 15
#endif
#ifndef ECTF_NOTFUNC
#define ECTF_NOTFUNC 1001
#endif
#ifndef ENOMEM
#define ENOMEM 12
#endif

/* Minimal type definitions to compile */
typedef unsigned long ctf_id_t;

typedef struct ctf_dynhash ctf_dynhash_t;

typedef struct ctf_dict {
    ctf_dynhash_t *ctf_funchash;
    ctf_dynhash_t *ctf_objthash;
} ctf_dict_t;

/* External functions used (will be stubbed or auto-stubbed) */

