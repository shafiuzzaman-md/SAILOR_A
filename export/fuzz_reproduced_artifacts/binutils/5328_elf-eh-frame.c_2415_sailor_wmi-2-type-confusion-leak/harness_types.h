/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdbool.h>
#include <stddef.h>

// Minimal forward declarations to satisfy signatures
typedef struct bfd bfd;
struct bfd_link_info { int dummy; };
typedef struct asection asection;

// Minimal types to reach the vulnerable field access
struct eh_frame_hdr_info { int dummy; };
struct elf_link_hash_table {
    struct eh_frame_hdr_info eh_info;  // must exist for vulnerable access
};

// External function that will be stubbed

