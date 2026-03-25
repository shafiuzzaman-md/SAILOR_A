/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#ifndef SEPOL_OK
#define SEPOL_OK 0
#endif
#ifndef SEPOL_ERR
#define SEPOL_ERR -1
#endif
#ifndef CIL_ROLETYPE
#define CIL_ROLETYPE 42
#endif

// Minimal type models needed for the path
struct cil_db { int dummy; };

struct cil_roletype {
    char *role_str;
    char *type_str;
};

struct cil_tree_node {
    struct cil_tree_node *next;
    void *data;
    int flavor;
};

