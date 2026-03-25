/* AUTO-GENERATED from harness preamble */
#pragma once

/* Minimal harness for _bfd_elf_strtab_finalize focusing on the vulnerable dereference. */
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#ifndef BFD_SIZE_TYPE
#define BFD_SIZE_TYPE size_t
#endif

typedef BFD_SIZE_TYPE bfd_size_type;

/* Minimal struct definition to support the vulnerable access. */
struct elf_strtab_hash {
    size_t size;
};

/* Entry == Vulnerable function. Keep only the vulnerable statement and sink. */
