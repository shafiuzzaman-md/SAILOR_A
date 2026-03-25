/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#ifndef SYMNMLEN
#define SYMNMLEN 8
#endif
#ifndef STRING_SIZE_SIZE
#define STRING_SIZE_SIZE 4
#endif
#ifndef BFD_ASSERT
#define BFD_ASSERT(x) ((void)0)
#endif

/* Minimal forward decl for bfd to satisfy signature. */
typedef struct bfd bfd;

/* Minimal internal_syment definition with the required nesting. */
struct internal_syment {
  union {
    char _n_name[SYMNMLEN];
    struct {
      uint32_t _n_zeroes;
      uint32_t _n_offset;
    } _n_n;
  } _n;
};

/* Externals referenced in the else-path; will be auto-stubbed. */

/* Vulnerable function (entry == vulnerable). Keep the vulnerable statement verbatim. */
const char *
_bfd_coff_internal_syment_name (bfd *abfd,
                                const struct internal_syment *sym,
