/* AUTO-GENERATED from harness preamble */
#pragma once

/* Minimal sliced harness for binutils ELF path
 * Entry: _bfd_elf_compute_section_file_positions
 * Vuln:  _bfd_elf_map_sections_to_segments (contains vulnerable statement)
 */
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

/* Minimal stand-in types to satisfy signatures */
typedef struct bfd bfd;
struct bfd_link_info { int dummy; };

/* Fake bfd layout to support elf_seg_map(abfd) macro deref into tdata */
struct fake_bfd { void *tdata; };
struct elf_segment_map { int dummy; };
#define elf_seg_map(abfd) (*(struct elf_segment_map**)&(((struct fake_bfd*)(abfd))->tdata))

/* ENTRY FUNCTION (MANDATORY neutralized pass-through) */

