/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifndef SEPOL_OK
#define SEPOL_OK 0
#endif
#ifndef SEPOL_ERR
#define SEPOL_ERR -1
#endif
#ifndef CIL_ROLEALLOW
#define CIL_ROLEALLOW 1001
#endif

// Minimal type models to reach the vulnerable statement
struct cil_tree_node {
    struct cil_tree_node *next;
    void *data;
    int flavor;
};

struct cil_roleallow {
    char *src_str;
    char *tgt_str;
};

struct cil_db { int dummy; };

