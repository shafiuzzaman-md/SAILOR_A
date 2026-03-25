/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#ifndef ATTRIBUTE_UNUSED
#define ATTRIBUTE_UNUSED
#endif

#ifndef ELFCLASS32
#define ELFCLASS32 1
#endif

/* Minimal type definitions needed by the sliced functions */
typedef uint64_t bfd_vma;

typedef struct bfd {
  void *xvec; /* opaque in this slice */
} bfd;

struct elf_size_info {
  int elfclass;
};

struct elf_backend_data {
  const struct elf_size_info *s;
};

/* External helpers (stubbed in stubs.c) */

