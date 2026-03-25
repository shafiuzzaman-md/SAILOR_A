/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

// Minimal forward declarations to avoid pulling full project headers
typedef struct bfd bfd;

// Minimal map entry structure to host the vulnerable assignment
struct map_entry_u {
    bfd *abfd;
};

typedef struct {
    char **name;
    struct map_entry_u u;
    unsigned int namidx;
} map_entry;

// VUL prototype

