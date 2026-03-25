#include "harness_types.h"
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <klee/klee.h>

// Declaration from harness
int harness_entry(VC1Context *v);

int main() {
    VC1Context *v = (VC1Context *)calloc(1, sizeof(VC1Context));
    if (!v) return 0;

    // Allocate block_index with at least 1 element and set it to 0 so (idx - 1) becomes -1
    int *block_index = (int *)malloc(sizeof(int) * 1);
    if (!block_index) return 0;
    block_index[0] = 0;
    v->s.block_index = block_index;

    // Allocate mb_type[0] with a small concrete size (the -1 access will be OOB)
    v->mb_type[0] = (uint8_t *)malloc(4);
    if (!v->mb_type[0]) return 0;
    // Make contents symbolic but force element 0 to be non-zero so the second operand is evaluated
    klee_make_symbolic(v->mb_type[0], 4, "mb_type0_buf");
    v->mb_type[0][0] = 1;

    // Call entry (pure pass-through to vulnerable function)
    int ret = harness_entry(v);
    return ret;
}
