/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

// Minimal local stand-ins to satisfy signatures/types used in the harness
typedef struct bfd bfd;                  // opaque to harness
struct bfd_link_info;                    // opaque to harness

// Minimal internal program header used by the vulnerable code
typedef struct {
    uint64_t p_vaddr;
    uint64_t p_paddr;
    uint32_t p_type;
    uint64_t p_filesz;
    uint64_t p_memsz;
    uint32_t p_flags;
} Elf_Internal_Phdr;

// Pieces referenced around the vulnerable site
struct elf_segment_map_flags { int p_paddr_valid; };
struct sorted_seg_map_ent { int includes_filehdr; int idx; };

// Forward decl

