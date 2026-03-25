/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

// Minimal local types to satisfy signatures
struct coff_final_link_info;  // opaque to harness

// Minimal PE data structures to satisfy dores_com assignments
struct pe_optional_header {
    unsigned int SizeOfHeapReserve;
    unsigned int SizeOfStackReserve;
    unsigned int SizeOfHeapCommit;
    unsigned int SizeOfStackCommit;
};

struct pe_data { struct pe_optional_header pe_opthdr; };

typedef struct bfd {
    void *pe;  // points to struct pe_data
} bfd;

#define pe_data(abfd) ((struct pe_data *) ((abfd)->pe))

// Globals provided by driver
extern char *g_ptr;
extern bfd *g_bfd;
extern int g_heap;

