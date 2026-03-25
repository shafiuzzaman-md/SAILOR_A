/* AUTO-GENERATED from harness preamble */
#pragma once

/* Minimal sliced harness for _bfd_x86_elf_late_size_sections -> _bfd_x86_elf_write_sframe_plt */
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

#ifndef BFD_ASSERT
#define BFD_ASSERT(x) ((void)0)
#endif

/* Minimal type stubs needed by the harness */
typedef struct bfd bfd; /* opaque */

typedef unsigned long bfd_size_type;

struct asection {
    bfd_size_type size;
    unsigned char *contents;
};
typedef struct asection asection;

struct bfd_link_info { int dummy; };

/* Backend data with a target id field referenced by original code */
struct elf_backend_data { unsigned int target_id; };

/* SFrame encoder ctx (opaque) */
typedef struct sframe_encoder_ctx sframe_encoder_ctx;

/* Hash table used on path with only needed fields */
struct elf_x86_link_hash_table {
    struct { bfd *dynobj; } elf;
    sframe_encoder_ctx *plt_cfe_ctx;
    sframe_encoder_ctx *plt_second_cfe_ctx;
    sframe_encoder_ctx *plt_got_cfe_ctx;
    asection *plt_sframe;
    asection *plt_second_sframe;
    asection *plt_got_sframe;
};

/* PLT section type enum values */
#ifndef SFRAME_PLT
#define SFRAME_PLT      0
#endif
#ifndef SFRAME_PLT_SEC
#define SFRAME_PLT_SEC  1
#endif
#ifndef SFRAME_PLT_GOT
#define SFRAME_PLT_GOT  2
#endif

/* External functions to be stubbed */

/* VULNERABLE FUNCTION — keep only target case body and vulnerable statement verbatim */
static bool _bfd_x86_elf_write_sframe_plt (bfd *output_bfd,
                               struct bfd_link_info *info,
