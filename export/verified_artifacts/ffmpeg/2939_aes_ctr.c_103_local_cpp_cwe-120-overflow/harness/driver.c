#include "harness_types.h"
#include <stdint.h>
#include <stdlib.h>
#include <klee/klee.h>

int entry_func(struct AVAESCTR *a);

int main() {
    struct AVAESCTR *a = (struct AVAESCTR *)calloc(1, sizeof(struct AVAESCTR));
    if (!a) return 0;

    // Counter buffer: 16 bytes so increment_be64 stays in-bounds; memset at +16 goes OOB.
    uint8_t *ctr = (uint8_t *)malloc(16);
    if (!ctr) return 0;
    klee_make_symbolic(ctr, 16, "counter_bytes");

    a->counter = ctr;
    a->block_offset = 0;

    // No need to make encrypted_counter symbolic for this path.

    entry_func(a);
    return 0;
}
