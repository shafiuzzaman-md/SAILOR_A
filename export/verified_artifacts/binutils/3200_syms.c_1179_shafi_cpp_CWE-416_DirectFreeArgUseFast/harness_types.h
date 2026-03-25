/* AUTO-GENERATED from harness preamble */
#pragma once

/* harness/spine.c */
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#ifndef KLEE_USE_STDLIB
#define KLEE_USE_STDLIB 1
#endif

/* Minimal typedefs to satisfy signature */
typedef unsigned long bfd_vma;
typedef unsigned long bfd_size_type;
typedef unsigned char bfd_byte;
typedef struct bfd { int dummy; } bfd;
typedef struct asymbol { int dummy; } asymbol;
typedef struct asection { int dummy; } asection;

/* From syms.c preamble (needed for *pinfo) */
struct indexentry {
  bfd_vma val;
  bfd_byte *stab;
  bfd_byte *str;
  char *directory_name;
  char *file_name;
  char *function_name;
  int idx;
};

struct stab_find_info {
  asection *stabsec;
  asection *strsec;
  bfd_byte *stabs;
  bfd_byte *strs;
  struct indexentry *indextable;
  int indextablesize;
#ifdef ENABLE_CACHING
  struct indexentry *cached_indexentry;
  bfd_vma cached_offset;
  bfd_byte *cached_stab;
  char *cached_file_name;
#endif
  char *filename;
};

/* Vulnerable function (entry == vulnerable). Neutralized body keeping the free-site verbatim and a UAF use. */
bool _bfd_stab_section_find_nearest_line(bfd *abfd,
                                         asymbol **symbols,
                                         asection *section,
                                         bfd_vma offset,
                                         bool *pfound,
                                         const char **pfilename,
                                         const char **pfnname,
                                         unsigned int *pline,
                                         void **pinfo)
{
    (void)abfd; (void)symbols; (void)section; (void)offset;
    (void)pfound; (void)pfilename; (void)pfnname; (void)pline;

