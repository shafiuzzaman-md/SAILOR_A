/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>

#ifndef UNUSED
#define UNUSED(x) (void)(x)
#endif

/* Minimal type and macro definitions to reach the vulnerable statement. */
typedef uint32_t ctf_id_t;

typedef struct ctf_dict {
    void *ctf_dictops; /* field intentionally modeled to force deref of fp */
} ctf_dict_t;

typedef struct ctf_type {
    uint32_t ctt_info;
} ctf_type_t;

/* Callback type as used by ctf_type_visit/ctf_type_rvisit. */
typedef int ctf_visit_f (const char *name, ctf_id_t type, unsigned long offset,
                         int depth, void *arg);

/* Model LCTF_INFO_KIND so that it dereferences fp (use-after-free read site). */
#ifndef LCTF_INFO_KIND
#include <stdint.h>
#define LCTF_INFO_KIND(fp, info) ((uint32_t)(uintptr_t)((fp)->ctf_dictops))
#endif

