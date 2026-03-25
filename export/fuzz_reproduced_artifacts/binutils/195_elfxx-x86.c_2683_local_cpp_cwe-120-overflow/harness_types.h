/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

/* Minimal type stand-ins to satisfy the vulnerable statement */
typedef struct bfd { int dummy; } bfd;
struct bfd_link_info { int dummy; };

typedef struct asection {
    unsigned char *contents;
    size_t size;
    struct asection *next;
} asection;

struct plt_section { unsigned char *eh_frame_plt; };

struct elf_sub { asection *splt; };

struct elf_x86_link_hash_table {
    asection *plt_eh_frame;
    asection *plt_got_eh_frame;
    asection *plt_got;
    struct elf_sub elf;
    struct plt_section *non_lazy_plt;
    struct { unsigned char *eh_frame_plt; } plt;
};

/* Global htab to be initialized by the driver */
struct elf_x86_link_hash_table *global_htab;

