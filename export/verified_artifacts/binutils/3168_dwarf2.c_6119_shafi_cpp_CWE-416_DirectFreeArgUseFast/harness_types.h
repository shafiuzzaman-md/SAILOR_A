/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Minimal opaque type declarations to satisfy signatures */
typedef struct bfd bfd;
typedef struct asymbol asymbol;
typedef struct asection asection;
typedef unsigned long bfd_vma;
struct dwarf_debug_section;

/* Minimal structures to exercise the vulnerable path */
struct line_table {
    char *files;
    char *dirs;
};

struct comp_unit {
    struct line_table *line_table;
    struct comp_unit *next_unit;
};

struct dwarf2_file {
    struct comp_unit *all_comp_units;
    struct line_table *line_table;
};

/* Entry function: STRICT pass-through to vulnerable function (no guards) */
int _bfd_dwarf2_find_nearest_line(bfd *abfd,
                                  asymbol **symbols,
                                  asymbol *symbol,
                                  asection *section,
                                  bfd_vma offset,
                                  const char **filename_ptr,
                                  const char **functionname_ptr,
                                  unsigned int *linenumber_ptr,
                                  unsigned int *discriminator_ptr,
                                  const struct dwarf_debug_section *debug_sections,
                                  void **pinfo) {
    _bfd_dwarf2_find_nearest_line_with_alt(abfd, NULL, symbols, symbol, section, offset,
                                           filename_ptr, functionname_ptr,
                                           linenumber_ptr, discriminator_ptr,
                                           debug_sections, pinfo);
    return 0;
}

/* Vulnerable function: keep signature, neutralized body with exact vulnerable line */
int _bfd_dwarf2_find_nearest_line_with_alt(bfd *abfd,
                                           const char *alt_filename,
                                           asymbol **symbols,
                                           asymbol *symbol,
                                           asection *section,
                                           bfd_vma offset,
                                           const char **filename_ptr,
                                           const char **functionname_ptr,
                                           unsigned int *linenumber_ptr,
                                           unsigned int *discriminator_ptr,
                                           const struct dwarf_debug_section *debug_sections,
                                           void **pinfo) {
    (void)abfd; (void)alt_filename; (void)symbols; (void)symbol; (void)section; (void)offset;
    (void)filename_ptr; (void)functionname_ptr; (void)linenumber_ptr; (void)discriminator_ptr;
    (void)debug_sections; (void)pinfo;

    /* Minimal reconstruction of objects needed to reach the vulnerable free() lines. */
    struct dwarf2_file *file = (struct dwarf2_file *)calloc(1, sizeof(struct dwarf2_file));
    struct comp_unit *each = (struct comp_unit *)calloc(1, sizeof(struct comp_unit));
    file->line_table = (struct line_table *)calloc(1, sizeof(struct line_table));
    each->line_table = (struct line_table *)calloc(1, sizeof(struct line_table));

    /* Allocate inner buffers so the frees below act on real heap objects. */
    each->line_table->files = (char *)malloc(64);
    each->line_table->dirs  = (char *)malloc(64);

