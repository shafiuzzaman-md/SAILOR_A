/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#ifndef SEPOL_OK
#define SEPOL_OK 0
#endif
#ifndef SEPOL_EEXIST
#define SEPOL_EEXIST 17
#endif

struct cil_complex_symtab_key {
    uint32_t key1;
    uint32_t key2;
    uint32_t key3;
    uint32_t key4;
};

struct cil_complex_symtab_datum { int dummy; };

struct cil_complex_symtab_node {
    struct cil_complex_symtab_key *ckey;
    struct cil_complex_symtab_datum *datum;
    struct cil_complex_symtab_node *next;
};

struct cil_complex_symtab {
    struct cil_complex_symtab_node **htable;
    uint32_t nelems;
    uint32_t nslots;
    uint32_t mask;
};
