/* AUTO-GENERATED from harness preamble */
#pragma once

/* harness/spine.c - minimal neutralized harness for objalloc_free */
#include <stdlib.h>

/* Minimal struct definitions to satisfy the path */
struct objalloc_chunk {
    struct objalloc_chunk *next;
    char *current_ptr;
};

struct objalloc {
    void *chunks; /* original code casts this to struct objalloc_chunk* */
};

