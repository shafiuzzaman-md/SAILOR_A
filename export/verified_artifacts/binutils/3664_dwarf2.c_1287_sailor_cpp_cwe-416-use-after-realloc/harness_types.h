/* AUTO-GENERATED from harness preamble */
#pragma once

/* Minimal sliced harness for binutils dwarf2.c UAF in read_abbrevs */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdbool.h>

#ifndef ABBREV_HASH_SIZE
#define ABBREV_HASH_SIZE 121
#endif
#ifndef ATTR_ALLOC_CHUNK
#define ATTR_ALLOC_CHUNK 4
#endif
#ifndef INSERT
#define INSERT 0
#endif

/* Minimal opaque type definitions to satisfy signatures */
typedef unsigned long long bfd_vma;
typedef unsigned char bfd_byte;

typedef struct bfd { unsigned long flags; } bfd;
typedef struct asymbol { const char *name; } asymbol;
typedef struct asection { unsigned int id; } asection;
struct dwarf_debug_section { int dummy; };

struct abbrev_info {
    unsigned int number;
    unsigned int tag;
    unsigned int has_children;
    struct abbrev_info *next;
    void *attrs;
};

struct abbrev_offset_entry {
    size_t offset;
    struct abbrev_info **abbrevs;
};

struct dwarf2_debug_alt {
    bfd *bfd_ptr;
    void *syms;
    unsigned char *dwarf_str_buffer;
    size_t dwarf_str_size;
};

struct dwarf2_debug {
    struct dwarf2_debug_alt alt;
    const struct dwarf_debug_section *debug_sections;
};

struct dwarf2_debug_file {
    void *abbrev_offsets;   /* hashtable */
    unsigned char *dwarf_abbrev_buffer;
    size_t dwarf_abbrev_size;
    void *syms;
};

/* External helpers we stub in stubs.c */

/* Vulnerable function — neutralized body keeping the exact vulnerable line. */
static struct abbrev_info**
read_abbrevs (bfd *abfd, uint64_t offset, struct dwarf2_debug *stash,
