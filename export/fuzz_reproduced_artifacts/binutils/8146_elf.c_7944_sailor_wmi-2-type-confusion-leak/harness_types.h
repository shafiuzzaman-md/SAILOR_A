/* AUTO-GENERATED from harness preamble */
#pragma once

/* harness/spine.c - neutralized minimal harness */
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#ifndef BFD_VMA_TYPEDEF
#define BFD_VMA_TYPEDEF
typedef unsigned long bfd_vma;
#endif

typedef struct elf_internal_phdr {
  unsigned long p_type;
  unsigned long p_flags;
  bfd_vma      p_offset;
  bfd_vma      p_vaddr;
  bfd_vma      p_paddr;
  bfd_vma      p_filesz;
  bfd_vma      p_memsz;
  bfd_vma      p_align;
} Elf_Internal_Phdr;

struct elf_segment_map {
  struct elf_segment_map *next;
  unsigned long p_type;
  unsigned long p_flags;
  unsigned int  p_flags_valid;
  bfd_vma       p_paddr;
  bool          p_paddr_valid;
  int           includes_filehdr;
  int           includes_phdrs;
};

typedef struct bfd {
  Elf_Internal_Phdr *phdr;                /* models elf_tdata(ibfd)->phdr */
  struct elf_segment_map *segmap;         /* models elf_seg_map(obfd) */
} bfd;

/* VULNERABLE FUNCTION (neutralized to the core sink path) */
