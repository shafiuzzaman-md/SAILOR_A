/* AUTO-GENERATED from harness preamble */
#pragma once

/* Minimal sliced harness for ctf-archive.c vulnerability at line 285 */
#include <stdint.h>
#include <unistd.h>
#include <stddef.h>

// Minimal type needed on the path
typedef struct ctf_dict {
    int ctf_errno;  // field read at the vulnerable site
} ctf_dict_t;

// Prototypes (match original signatures)

