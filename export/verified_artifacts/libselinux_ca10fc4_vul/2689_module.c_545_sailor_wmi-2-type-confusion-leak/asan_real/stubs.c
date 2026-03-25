#include "harness_types.h"
#include <stdlib.h>
#include <stddef.h>

// Force error path in harness/module.c
void *my_malloc(size_t sz) {
    (void)sz;
    return NULL; // trigger the !mod->seusers path
}
