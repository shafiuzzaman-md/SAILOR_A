/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#ifndef BFD_VMA_DEFINED
#define BFD_VMA_DEFINED
typedef unsigned long bfd_vma;
#endif

typedef unsigned char bfd_byte;

typedef struct bfd { int dummy; } bfd;

typedef struct asymbol {
    const char *name;
} asymbol;

typedef struct arelent {
    bfd_vma address;
    asymbol **sym_ptr_ptr;
} arelent;

typedef struct fake_sec { bfd_vma vma; } fake_sec;

struct elf_x86_plt {
    fake_sec *sec;
    bfd_vma plt_got_insn_size;
    int count;
};
