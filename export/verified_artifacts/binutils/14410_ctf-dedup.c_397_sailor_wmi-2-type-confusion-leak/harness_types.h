/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <errno.h>

// Minimal project types needed by harness
typedef struct ctf_dedup_s {
    int dummy;
} ctf_dedup_t;

typedef struct ctf_dict_s {
    int ctf_link_flags;
    void *ctf_dedup_atoms;   // accessed by intern()
    ctf_dedup_t ctf_dedup;   // referenced by entry signature
} ctf_dict_t;

// External stubs provided separately

// Driver-provided atom pointer
extern char *HAR_atom;

