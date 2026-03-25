/* AUTO-GENERATED from harness preamble */
#pragma once

/* Minimal sliced harness for _bfd_pei_final_link_postscript -> rsrc_process_section */
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

/* Minimal typedefs/structs to satisfy signatures and fields used */
typedef uint64_t bfd_vma;
typedef size_t bfd_size_type;
typedef unsigned char bfd_byte;

typedef struct bfd { int dummy; } bfd;

typedef struct asection {
    bfd_vma vma;
    bfd_vma rawsize;
    bfd_vma size;
    struct asection *output_section;
    bfd_vma output_offset;
} asection;

typedef struct {
    struct { 
        uint64_t ImageBase; 
        struct { uint32_t VirtualAddress; } DataDirectory[16];
    } pe_opthdr;
} pe_data_type;

struct bfd_link_info { int dummy; };

struct coff_final_link_info {
    struct bfd_link_info *info;
    bfd *output_bfd;
};

/* Local helper struct approximating what write_data likely looks like */
typedef struct rsrc_write_data_s {
    bfd *abfd;
    bfd_byte *datastart;
    bfd_byte *next_table;
    bfd_byte *next_leaf;
    bfd_byte *next_string;
    bfd_byte *next_data;
    bfd_vma rva_bias;
} rsrc_write_data;

typedef struct { int dummy; } rsrc_directory;

/* Vulnerable function (neutralized to the minimal path; keep vulnerable statement verbatim) */
static void
rsrc_process_section (bfd * abfd,
