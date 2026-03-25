/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdlib.h>
#include <stdbool.h>

// Minimal local typedefs to satisfy signatures
typedef void* bfd_cleanup;

typedef struct bfd {
    void *external_syms;
    void *strings;
    unsigned long strings_len;
    unsigned long flags;
    unsigned int symcount;
    unsigned long start_address;
    int is_linker_input;
} bfd;

struct internal_filehdr { unsigned int f_flags; unsigned int f_nsyms; };
struct internal_aouthdr { unsigned long entry; };

// Macros mimicking accessors used by the vulnerable function
#define obj_coff_external_syms(abfd) ((abfd)->external_syms)
#define obj_coff_strings(abfd)       ((abfd)->strings)
#define obj_coff_strings_len(abfd)   ((abfd)->strings_len)
#define obj_coff_keep_syms(abfd)     (0)
#define obj_coff_keep_strings(abfd)  (0)
#define bfd_family_coff(abfd)        (1)

