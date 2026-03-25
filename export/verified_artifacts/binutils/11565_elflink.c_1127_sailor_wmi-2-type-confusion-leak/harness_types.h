/* AUTO-GENERATED from harness preamble */
#pragma once

/* Minimal sliced harness for elflink.c vulnerability
 * Entry: bfd_elf_link_add_symbols
 * Vuln: _bfd_elf_merge_symbol (bind = ELF_ST_BIND (sym->st_info);)
 */
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

/* Minimal type stand-ins (only what we touch) */
typedef uint64_t bfd_vma;

typedef struct bfd { int dummy; } bfd;
typedef struct bfd_link_info { int dummy; } bfd_link_info;
typedef struct asection { unsigned long flags; } asection;

typedef struct elf_link_hash_entry { int versioned; unsigned char other; } elf_link_hash_entry;

typedef struct Elf_Internal_Sym { unsigned char st_info; } Elf_Internal_Sym;

/* ELF helper stub: modeled as a function so the call syntax matches */

/* Globals to bridge driver -> entry -> vulnerable function */
extern const char *g_name;
extern Elf_Internal_Sym *g_sym;
extern asection *g_psec;
extern bfd_vma g_value;
extern struct elf_link_hash_entry *g_sym_hash;
extern bfd *g_oldbfd;
extern bool g_old_weak;
extern unsigned int g_old_alignment;
extern bool g_skip;
extern bfd *g_override;
extern bool g_type_change_ok;
extern bool g_size_change_ok;
extern bool g_matched;

/* Vulnerable function: keep signature and only the minimal path with the vulnerable statement */
static bool _bfd_elf_merge_symbol (bfd *abfd,
                                   struct bfd_link_info *info,
                                   const char *name,
                                   Elf_Internal_Sym *sym,
                                   asection **psec,
                                   bfd_vma *pvalue,
                                   struct elf_link_hash_entry **sym_hash,
                                   bfd **poldbfd,
                                   bool *pold_weak,
                                   unsigned int *pold_alignment,
                                   bool *skip,
                                   bfd **override,
                                   bool *type_change_ok,
                                   bool *size_change_ok,
                                   bool *matched)
{
    asection *sec;
    int bind;

    /* Neutralized preamble from the real function */
    *skip = false;
    *override = NULL;

    sec = *psec;
    /* VULNERABLE STATEMENT (verbatim) */
    bind = ELF_ST_BIND (sym->st_info);
    /* Universal sink assertion to mark reachability if it doesn't crash */

    (void)abfd; (void)info; (void)name; (void)pvalue; (void)sym_hash; (void)poldbfd;
    (void)pold_weak; (void)pold_alignment; (void)type_change_ok; (void)size_change_ok;
    (void)matched; (void)sec; (void)bind;
    return true;
}

/* ENTRY: strict pass-through calling the vulnerable function directly */
