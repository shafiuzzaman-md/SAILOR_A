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
#ifndef CIL_CLASSCOMMON
#define CIL_CLASSCOMMON 7
#endif

struct cil_db { int dummy; };

struct cil_tree_node {
    struct cil_tree_node *next;
    void *data;
    int flavor;
};

struct cil_classcommon {
    char *class_str;
    char *common_str;
};

