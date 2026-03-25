/* AUTO-GENERATED from harness preamble */
#pragma once

/* Minimal sliced harness for dwarf2.c vulnerability at line 2843 */
#include <stddef.h>
#include <stdint.h>

/* Minimal typedefs to satisfy signatures */
typedef long bfd_signed_vma;
typedef struct bfd bfd;
typedef unsigned char bfd_byte;
typedef unsigned long bfd_size_type;

/* asymbol and section skeletons to match entry signature */
struct bfd_section_s { bfd_signed_vma vma; };
typedef struct asymbol {
    unsigned int flags;
    struct bfd_section_s *section;
    bfd_signed_vma value;
    const char *name;
} asymbol;

/* Forward decl of external alloc used by vulnerable function */

/* Minimal structures used on the vulnerable path */
struct line_info_table { int dummy; };
struct dwarf2_debug { int dummy; };
struct dwarf2_debug_file { int dummy; };

struct comp_unit {
    bfd *abfd;
    struct dwarf2_debug *stash;
    struct dwarf2_debug_file *file;
};

/* Local copy of the header used in decode_line_info around the sink */
struct line_head {
    unsigned char *standard_opcode_lengths;
    unsigned int opcode_base;
};

/* Vulnerable function — keep only the relevant slice and the exact sink line */
