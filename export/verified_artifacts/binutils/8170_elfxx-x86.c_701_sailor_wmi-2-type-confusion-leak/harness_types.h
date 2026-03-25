/* AUTO-GENERATED from harness preamble */
#pragma once

/* Minimal sliced harness for elfxx-x86.c vulnerability @ line 701 */
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

/* Minimal type stubs to match access pattern */
struct bfd_link_hash_table { int _dummy; };

struct bfd_link_info_stub { void *hash; };

typedef struct bfd {
    struct bfd_link_info_stub link;
} bfd;

struct elf_x86_link_hash_table {
    void *loc_hash_memory;  /* vulnerable field read */
};

struct objalloc; /* opaque */

