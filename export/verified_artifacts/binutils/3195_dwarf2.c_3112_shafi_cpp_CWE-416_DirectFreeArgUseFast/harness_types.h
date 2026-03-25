/* AUTO-GENERATED from harness preamble */
#pragma once

/* Minimal sliced harness for binutils/bfd dwarf2.c UAF at line 3112 */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Minimal type shims */
typedef unsigned long bfd_vma;
typedef long bfd_signed_vma;
typedef unsigned char bfd_byte;

struct bfd { int dummy; };
struct dwarf2_debug { int dummy; };
struct dwarf2_debug_file { int dummy; };

struct asection { bfd_vma vma; };
typedef struct asymbol { const char *name; unsigned int flags; struct asection *section; bfd_vma value; } asymbol;
#ifndef BSF_FUNCTION
#define BSF_FUNCTION 0x10
#endif

struct funcinfo { const char *name; struct { bfd_vma low; } arange; struct funcinfo *prev_func; };

struct comp_unit {
    struct bfd *abfd;
    struct dwarf2_debug *stash;
    struct dwarf2_debug_file *file;
    unsigned int line_offset;
    struct comp_unit *next_unit;
    struct funcinfo *function_table;
};

struct line_sequence { int dummy; };
struct line_info { int dummy; };

struct line_info_table {
    /* Only fields referenced around the sink and in adjacent helpers */
    char **files;
    char **dirs;
    struct line_sequence *sequences;
};

/* Forward decls (not actually used in this slice, but keep signatures consistent) */

