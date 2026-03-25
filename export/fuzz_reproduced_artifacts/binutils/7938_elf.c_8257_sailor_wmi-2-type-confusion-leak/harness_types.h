/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

// Minimal type universe for the harness
typedef uint64_t bfd_vma;

#ifndef PT_INTERP
#define PT_INTERP 3
#endif
#ifndef PT_DYNAMIC
#define PT_DYNAMIC 2
#endif

typedef struct Elf_Internal_Phdr {
    bfd_vma p_paddr;
    bfd_vma p_memsz;
    unsigned int p_type;
} Elf_Internal_Phdr;

typedef struct {
    unsigned int e_phnum;
} Elf_Internal_Ehdr;

typedef struct elf_obj_tdata {
    Elf_Internal_Phdr *phdr;
    Elf_Internal_Ehdr elf_header;
} elf_obj_tdata;

typedef struct bfd_section {
    struct bfd_section *next;
    struct bfd_section *output_section;
    bool segment_mark;
    bfd_vma lma, vma;
} asection;

typedef struct bfd {
    void *xvec;
    asection *sections;
    void *tdata; // points to elf_obj_tdata
} bfd;

#define elf_tdata(abfd) ((elf_obj_tdata *) ((abfd)->tdata))
#define elf_elfheader(abfd) (&(elf_tdata(abfd)->elf_header))

