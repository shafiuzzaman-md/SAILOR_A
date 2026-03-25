/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

/* Minimal type definitions needed by the harness */
typedef unsigned char bfd_byte;
typedef size_t bfd_size_type;

typedef struct bfd { int dummy; } bfd;

typedef struct asection {
    bfd_size_type size;
    unsigned int flags;
    int sec_info_type;
    struct asection *output_section;
} asection;

struct bfd_link_info { int dummy; };
struct elf_reloc_cookie { int dummy; };

/* SFrame decoder related minimal types */
typedef struct sframe_decoder_ctx { int dummy; } sframe_decoder_ctx;
struct sframe_dec_info { sframe_decoder_ctx *sfd_ctx; };

/* Local macro definitions (guessed reasonable defaults) */
#ifndef SEC_HAS_CONTENTS
#define SEC_HAS_CONTENTS 0x1
#endif
#ifndef SEC_INFO_TYPE_NONE
#define SEC_INFO_TYPE_NONE 0
#endif
#ifndef SEC_INFO_TYPE_SFRAME
#define SEC_INFO_TYPE_SFRAME 1
#endif

/* External helpers to be provided by stubs.c */

/* Vulnerable function (entry == vulnerable). Neutralized to keep only the direct path
   and the vulnerable statement verbatim, followed by the universal sink assertion. */
bool _bfd_elf_parse_sframe(bfd *abfd,
                           struct bfd_link_info *info /* ATTRIBUTE_UNUSED */,
