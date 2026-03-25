/* AUTO-GENERATED from harness preamble */
#pragma once

/* Minimal sliced harness for ctf_lookup_by_name → ctf_lookup_by_name_internal */
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#ifndef ENOMEM
#define ENOMEM 12
#endif
#ifndef EINVAL
#define EINVAL 22
#endif
#ifndef ECTF_SYNTAX
#define ECTF_SYNTAX 1001
#endif

/* Minimal typedefs/structs needed by the path */
typedef long ctf_id_t;

typedef struct ctf_dict_s {
    size_t ctf_tmp_typeslicelen;
    char *ctf_tmp_typeslice;
} ctf_dict_t;

typedef struct ctf_lookup_s {
    void *ctl_hash;
    const char *ctl_prefix;
} ctf_lookup_t;

/* External helpers to be stubbed */

