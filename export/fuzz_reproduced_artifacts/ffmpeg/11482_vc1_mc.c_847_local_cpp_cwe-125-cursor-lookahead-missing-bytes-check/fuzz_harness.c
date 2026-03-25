#include <stddef.h>
// Combined reproducer for 11482_vc1_mc.c_847_local_cpp_cwe-125-cursor-lookahead-missing-bytes-check
// Original harness: driver.c + smart_stubs.c + sliced source

// === smart_stubs.c ===
/* Smart stubs — auto-generated from path + vulnerability analysis */
/* Symbolic stubs model the environment: KLEE explores return values */
/* that both REACH the sink AND TRIGGER the vulnerability */
#include <stdlib.h>
#include <string.h>
/* PROACTIVE: FUNCTION (auto-detected external) */
int FUNCTION() { return 0; }

/* PROACTIVE: source (auto-detected external) */
int source() { return 0; }

/* PROACTIVE: through (auto-detected external) */
int through() { return 0; }

// === driver.c ===
#include "harness_types.h"
#include <stdint.h>
#include <stdlib.h>
// entry_func is defined in harness/vc1_mc.c
int entry_func(VC1Context *v, int dir, int dir2, int avg);

int LLVMFuzzerTestOneInput(const uint8_t *fuzz_data, size_t fuzz_size) {
    if (fuzz_size < 64) return 0;
    // Allocate VC1Context concretely
    VC1Context *v = (VC1Context *)calloc(1, sizeof(VC1Context));

    // Allocate blk_mv_type with a small fixed size (1) to make OOB easier
    int *blk_mv_type = (int *)malloc(sizeof(int) * 1);
    // Content can be anything; make it symbolic but size is concrete
    memcpy(blk_mv_type, fuzz_data + (0), sizeof(int) * 1);
    v->blk_mv_type = blk_mv_type;

    // Allocate block_index array with at least one element
    int *block_index = (int *)malloc(sizeof(int) * 1);
    // Make the first index symbolic and force it to be out-of-bounds for len=1
    int idx0;
    memcpy(&idx0, fuzz_data + (sizeof(int) * 1), sizeof(idx0));
     // any non-zero index will be OOB for size 1
    block_index[0] = idx0;
    v->s.block_index = block_index;

    // Other fields not needed by harness, but initialize to safe defaults
    v->s.v_edge_pos = 0;

    // dir/dir2/avg are not used in neutralized harness; keep them symbolic
    int dir, dir2, avg;
    memcpy(&dir, fuzz_data + (sizeof(int) * 1 + sizeof(idx0)), sizeof(dir));
    memcpy(&dir2, fuzz_data + (sizeof(int) * 1 + sizeof(idx0) + sizeof(dir)), sizeof(dir2));
    memcpy(&avg, fuzz_data + (sizeof(int) * 1 + sizeof(idx0) + sizeof(dir) + sizeof(dir2)), sizeof(avg));

    // Call entry (pure pass-through)
    entry_func(v, dir, dir2, avg);
    return 0;
}
