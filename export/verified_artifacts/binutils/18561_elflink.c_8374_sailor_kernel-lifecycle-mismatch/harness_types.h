/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdlib.h>
#include <stdint.h>

#ifndef ARCH_SIZE
#define ARCH_SIZE 0
#endif

/* Minimal type mirrors needed by the spine. */
typedef struct bfd {
    struct { void *hash; } link;
} bfd;

struct dynamic_dummy { void *contents; };

struct elf_link_hash_table {
    void *dynstr;
    void *merge_info;
    struct dynamic_dummy *dynamic;
    void *first_hash;
};

/* Externs that the spine calls. Implemented in stubs.c. */

/* Destroy an ELF linker hash table.  */

void
