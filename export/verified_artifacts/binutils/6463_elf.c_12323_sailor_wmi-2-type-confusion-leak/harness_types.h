/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdlib.h>
#include <stdint.h>

/* Minimal type definitions to exercise the path */
struct elf_backend_data {
    int linux_prpsinfo32_ugid16;
};

typedef struct bfd {
    struct elf_backend_data *xvec; /* backend data (target vector) */
} bfd;

struct elf_internal_linux_prpsinfo { int dummy; };
