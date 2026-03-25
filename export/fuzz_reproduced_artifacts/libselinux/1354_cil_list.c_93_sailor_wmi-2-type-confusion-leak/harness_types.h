/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdint.h>
#include <stdlib.h>

// Minimal enum and struct definitions needed by the slice
enum cil_flavor { CIL_NONE = 0, CIL_LIST = 1 };

struct cil_list_item {
    struct cil_list_item *next;
    enum cil_flavor flavor;
    void *data;
};

// External destructor from project code — we stub it in stubs.c

// Vulnerable function (neutralized to the target statement only)
