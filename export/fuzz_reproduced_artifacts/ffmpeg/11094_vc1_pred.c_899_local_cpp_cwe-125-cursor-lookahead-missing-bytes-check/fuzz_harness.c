#include <stddef.h>
// Combined reproducer for 11094_vc1_pred.c_899_local_cpp_cwe-125-cursor-lookahead-missing-bytes-check
// Original harness: driver.c + smart_stubs.c + sliced source

// === smart_stubs.c ===
/* Smart stubs — auto-generated from path + vulnerability analysis */
/* Symbolic stubs model the environment: KLEE explores return values */
/* that both REACH the sink AND TRIGGER the vulnerability */
#include <stdlib.h>
#include <string.h>
/* PROACTIVE: entry (auto-detected external) */
int entry() { return 0; }

/* PROACTIVE: function (auto-detected external) */
int function() { return 0; }

/* Removed invalid stub for int16_t (typedef). */

/* PROACTIVE: pointers (auto-detected external) */
int pointers() { return 0; }

// === driver.c ===
#include "harness_types.h"
#include <stdint.h>
#include <stdlib.h>
int LLVMFuzzerTestOneInput(const uint8_t *fuzz_data, size_t fuzz_size) {
    if (fuzz_size < 64) return 0;
    VC1Context *v = (VC1Context *)calloc(1, sizeof(VC1Context));
    if (!v) return 0;

    // Wire up picture pointers to internal storage arrays
    v->s.cur_pic.motion_val[0] = v->s.cur_mv0;
    v->s.cur_pic.motion_val[1] = v->s.cur_mv1;
    v->s.next_pic.motion_val[0] = v->s.next_mv0;
    v->s.next_pic.motion_val[1] = v->s.next_mv1;
    v->s.cur_pic.mb_type = v->s.mb_type_storage;
    v->s.next_pic.mb_type = v->s.mb_type_storage;

    // Set other innocuous fields
    v->s.quarter_sample = 0;
    v->bfraction = 0;

    // Make indices symbolic to let KLEE explore OOB on motion_val[1]
    int idx0, off;
    memcpy(&idx0, fuzz_data + (0), sizeof(idx0));
    memcpy(&off, fuzz_data + (sizeof(idx0)), sizeof(off));

    // Optionally bound them to a small neighborhood to help exploration
    
    

    v->s.block_index[0] = idx0;
    v->blocks_off = off;

    // Encourage the OOB path so KLEE hits the vulnerable read
    int sum = idx0 + off;
    

    // Call entry — pass-through to vulnerable function
    entry_func(v, 0, NULL, NULL, 0, NULL);
    return 0;
}
