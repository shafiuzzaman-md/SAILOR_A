/* AUTO-GENERATED from harness preamble */
#pragma once

/* harness/spine.c - minimal sliced harness */
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#ifndef NULL
#define NULL ((void*)0)
#endif

/* Minimal type skeletons needed by the harness */
typedef struct bfd bfd;  /* opaque in harness */
struct bfd_link_info { int dummy; };

struct elf_link_loaded_list {
    bfd *abfd;
    struct elf_link_loaded_list *next;
};

struct elf_link_hash_table {
    struct elf_link_loaded_list *dyn_loaded;
};

