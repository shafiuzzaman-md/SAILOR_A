/* AUTO-GENERATED from harness preamble */
#pragma once

/* Minimal sliced harness for _bfd_elf_compute_section_file_positions -> swap_out_syms */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Minimal typedefs/structs to satisfy the vulnerable statements */
typedef unsigned long bfd_vma;
typedef long file_ptr;

typedef struct bfd bfd;
struct bfd_target;
struct elf_backend_data;
struct elf_size_info;
struct bfd_link_info { int dummy; };

struct elf_size_info {
    unsigned int sizeof_sym;
    unsigned char log_file_align;
};

struct elf_backend_data {
    const struct elf_size_info *s;
    unsigned int elf_strtab_flags;
    void (*elf_backend_begin_write_processing)(bfd*, struct bfd_link_info*);
    bool (*elf_backend_init_file_header)(bfd*, struct bfd_link_info*);
};

struct bfd_target {
    const struct elf_backend_data *backend_data;
};

struct elf_strtab_hash { int dummy; };

/* Minimal ELF section header used by the vulnerable function */
typedef struct {
    uint32_t sh_type;
    uint64_t sh_entsize;
    uint64_t sh_size;
    uint32_t sh_info;
    bfd_vma  sh_addralign;
} Elf_Internal_Shdr;

/* Minimal per-bfd ELF tdata block exposing the headers used on path */
struct bfd_elf_data {
    Elf_Internal_Shdr symtab_hdr;
    Elf_Internal_Shdr strtab_hdr;
    Elf_Internal_Shdr shstrtab_hdr;
};

struct bfd {
    int flags;
    int output_has_begun;
    struct bfd_target *xvec;
    void *tdata; /* points to struct bfd_elf_data */
};

