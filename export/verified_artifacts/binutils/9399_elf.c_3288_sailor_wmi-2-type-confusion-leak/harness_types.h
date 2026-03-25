/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>

// Minimal local type defs sufficient for slicing
typedef struct bfd {
    void *build_id;  // referenced in real entry but unused in neutralized pass-through
} bfd;

typedef struct asection {
    unsigned long vma;
    unsigned long lma;
    unsigned long size;
} asection;

typedef struct {
    uint64_t p_type;
    uint64_t p_offset;
    uint64_t p_vaddr;
    uint64_t p_paddr;
    uint64_t p_filesz;
    uint64_t p_memsz;
    uint64_t p_align;
} Elf_Internal_Phdr;

// Stubs will be provided in stubs.c; declare prototypes used here

