/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

// Minimal type universe for this slice
typedef uint64_t bfd_size_type;
typedef uint64_t bfd_vma;
typedef unsigned char bfd_byte;
typedef long file_ptr;
typedef int bfd_boolean;

typedef struct bfd bfd;  // opaque for our slice
struct bfd_link_info { int dummy; };

struct coff_final_link_info {
    struct bfd_link_info *info;
    bfd *output_bfd;
};

typedef struct asection {
    bfd_size_type size;
    bfd_size_type rawsize;
    bfd_vma vma;
} asection;

typedef struct {
    struct { bfd_vma ImageBase; } pe_opthdr;
} pe_data_type;

// Decls used in vulnerable statements (definition provided in stubs)

// Entry: must be a direct pass-through to vulnerable function
