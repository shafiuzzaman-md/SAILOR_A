// Combined reproducer for 2939_aes_ctr.c_103_local_cpp_cwe-120-overflow
// Original harness: driver.c + smart_stubs.c + sliced source

// === smart_stubs.c ===
/* Smart stubs — auto-generated from path + vulnerability analysis */
/* Symbolic stubs model the environment: KLEE explores return values */
/* that both REACH the sink AND TRIGGER the vulnerability */
#include <stdlib.h>
#include <string.h>
#include <klee/klee.h>

// === driver.c ===
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
