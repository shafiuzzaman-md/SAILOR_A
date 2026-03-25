/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* Minimal typedefs to satisfy signature */
typedef unsigned long bfd_vma;
typedef size_t bfd_size_type;
typedef unsigned char bfd_byte;
typedef struct bfd { int _d; } bfd;
typedef struct asymbol { int _d; } asymbol;
typedef struct asection { int _d; } asection;

/* Structures from preamble needed on path */
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

/* Vulnerable function (also entry). Neutralized to keep only the UAF path. */
bool _bfd_stab_section_find_nearest_line(
    bfd *abfd,
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

