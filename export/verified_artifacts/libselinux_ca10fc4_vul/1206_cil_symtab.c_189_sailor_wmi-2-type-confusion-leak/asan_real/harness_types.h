/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#ifndef SEPOL_EEXIST
#define SEPOL_EEXIST 1
#endif

struct cil_complex_symtab_key {
    uint32_t key1;
    uint32_t key2;
    uint32_t key3;
    uint32_t key4;
};

struct cil_complex_symtab_node {
    struct cil_complex_symtab_key *ckey;
    void *datum;
    struct cil_complex_symtab_node *next;
};

struct cil_complex_symtab {
    size_t mask;  // assume mask = buckets-1
    struct cil_complex_symtab_node **htable;
};


