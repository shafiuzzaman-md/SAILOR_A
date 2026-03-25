/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>

// Minimal local type definitions needed by the harness
struct elf_obj_tdata;

typedef struct bfd {
    struct elf_obj_tdata *tdata;  // accessed via elf_tdata(abfd)
} bfd;

struct elf_obj_tdata {
    void *o;  // presence checked in original code
    void *dwarf2_find_line_info;
    void *dwarf1_find_line_info;
    void *line_info;
    struct { void *contents; } symtab_hdr;  // free (tdata->symtab_hdr.contents);
};

// External helpers (stubbed in stubs.c)

// Vulnerable function (spine). Keep signature and the vulnerable statement verbatim.
