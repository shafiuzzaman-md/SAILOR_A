/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#ifndef EXEC_P
#define EXEC_P 0x0002
#endif
#ifndef DYNAMIC
#define DYNAMIC 0x0040
#endif
#ifndef HAS_RELOC
#define HAS_RELOC 0x0100
#endif

// Minimal project types
typedef struct bfd {
  int flags;
  int output_has_begun;
} bfd;

struct bfd_link_info { int dummy; };

typedef struct {
  uint64_t sh_type;
  uint64_t sh_flags;
  uint64_t sh_addr;
  uint64_t sh_size;
  uint64_t sh_offset;
  uint64_t sh_entsize;
  uint64_t sh_link;
  uint64_t sh_info;
  uint64_t sh_addralign;
} Elf_Internal_Shdr;

// External stubs

// Vulnerable function (neutralized to keep only the target site)
