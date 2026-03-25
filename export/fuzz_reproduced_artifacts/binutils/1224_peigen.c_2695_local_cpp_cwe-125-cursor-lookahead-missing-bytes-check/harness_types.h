/* AUTO-GENERATED from harness preamble */
#pragma once

/* minimal sliced harness for pe_is_repro OOB read */
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

/* Minimal local type universe to compile the slice */
typedef unsigned long bfd_vma;
typedef unsigned long bfd_size_type;
typedef unsigned char bfd_byte;

/* Only the fields accessed on our kept path */
typedef struct {
    unsigned long VirtualAddress;
    unsigned long Size;
} IMAGE_DATA_DIRECTORY;

#define PE_DEBUG_DATA 6

struct internal_extra_pe_aouthdr {
    bfd_vma ImageBase;
    /* Deliberately small directory to provoke OOB on index 6 */
    IMAGE_DATA_DIRECTORY DataDirectory[1];
};

typedef struct {
    struct internal_extra_pe_aouthdr pe_opthdr;
    unsigned int real_flags;
} pe_data_type;

typedef struct asection {
    struct asection *next;
    bfd_vma vma;
    bfd_size_type size;
    unsigned int flags;
} asection;

typedef struct bfd {
    asection *sections;
} bfd;

/* External accessors provided by stubs */

/* === Neutralized vulnerable function: keep only the vulnerable statements === */
