/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#ifndef EH_FRAME_HDR_SIZE
#define EH_FRAME_HDR_SIZE 8
#endif
#ifndef SEC_INFO_TYPE_EH_FRAME
#define SEC_INFO_TYPE_EH_FRAME 1
#endif

typedef unsigned char bfd_byte;

typedef struct bfd { int dummy; } bfd;
typedef struct bfd_link_info { int dummy; } bfd_link_info;

typedef struct asection {
    int sec_info_type;
    void *sec_info;          // simplified: we store eh_frame_sec_info* here directly
    void *output_section;    // unused in harness
    unsigned int output_offset; // unused
    unsigned int size;       // unused
    unsigned int rawsize;    // unused
    void *owner;             // unused
} asection;

struct eh_cie_fde {
    unsigned int new_offset;
    unsigned int offset;
    unsigned int size;
    int removed;
};

struct eh_frame_sec_info {
    struct eh_cie_fde *entry;
    unsigned int count;
};

