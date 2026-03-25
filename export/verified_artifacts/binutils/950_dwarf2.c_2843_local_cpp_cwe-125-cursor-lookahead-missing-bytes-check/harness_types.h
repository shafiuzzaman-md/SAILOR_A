/* AUTO-GENERATED from harness preamble */
#pragma once

/* Minimal sliced harness for dwarf2.c vulnerability at line 2843
 * Spine: _bfd_dwarf2_find_symbol_bias -> decode_line_info
 */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifndef ABBREV_HASH_SIZE
#define ABBREV_HASH_SIZE 121
#endif
#ifndef ATTR_ALLOC_CHUNK
#define ATTR_ALLOC_CHUNK 4
#endif
#ifndef DIR_ALLOC_CHUNK
#define DIR_ALLOC_CHUNK 5
#endif
#ifndef FILE_ALLOC_CHUNK
#define FILE_ALLOC_CHUNK 5
#endif
#ifndef TRIE_LEAF_SIZE
#define TRIE_LEAF_SIZE 16
#endif

/* Minimal type shims */
typedef long bfd_signed_vma;
typedef unsigned char bfd_byte;
typedef struct bfd { int dummy; } bfd;

typedef struct asymbol_section { bfd_signed_vma vma; } asymbol_section;
typedef struct asymbol { unsigned int flags; asymbol_section *section; bfd_signed_vma value; const char *name; } asymbol;

struct comp_unit {
    bfd *abfd;
    unsigned char *buf;
    unsigned long buf_size;
    const char *comp_dir; /* not used in slice */
};

struct line_info_table { int dummy; };

struct line_head {
    unsigned int opcode_base;
    unsigned char *standard_opcode_lengths;
};

/* Decls for stubs provided in stubs.c */

