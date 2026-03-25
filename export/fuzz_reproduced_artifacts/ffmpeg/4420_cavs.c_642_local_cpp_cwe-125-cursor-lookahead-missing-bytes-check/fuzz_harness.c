#include <stdint.h>
#include <stddef.h>
// Combined reproducer for 4420_cavs.c_642_local_cpp_cwe-125-cursor-lookahead-missing-bytes-check
// Original harness: driver.c + smart_stubs.c + sliced source

// === smart_stubs.c ===
/* Smart stubs — auto-generated from path + vulnerability analysis */
/* Symbolic stubs model the environment: KLEE explores return values */
/* that both REACH the sink AND TRIGGER the vulnerability */
#include <stdlib.h>
#include <string.h>
/* PROACTIVE: function (auto-detected external) */
int function() { return 0; }

/* PROACTIVE: line (auto-detected external) */
int line() { return 0; }

// === driver.c ===
#include "harness_types.h"
#include <stdlib.h>

// Forward decl for the entry in harness
int entry_func(AVSContext *h);

int LLVMFuzzerTestOneInput(const uint8_t *fuzz_data, size_t fuzz_size) {
    if (fuzz_size < 64) return 0;
    AVSContext *h = (AVSContext *)calloc(1, sizeof(AVSContext));
    if (!h) return 0;

    // Concrete allocation sizes (no symbolic sizes)
    const int top_len = 8; // number of cavs_vector elements per top_mv plane
    h->top_mv[0] = (cavs_vector *)malloc(sizeof(cavs_vector) * top_len);
    h->top_mv[1] = (cavs_vector *)malloc(sizeof(cavs_vector) * top_len);
    if (!h->top_mv[0] || !h->top_mv[1]) return 0;

    // Make contents symbolic so KLEE explores all values
    memcpy(h->top_mv[0], fuzz_data + (0), sizeof(cavs_vector) * top_len);
    memcpy(h->top_mv[1], fuzz_data + (sizeof(cavs_vector) * top_len), sizeof(cavs_vector) * top_len);

    // Make scalar symbolic via local, then assign
    int mbx_sym = 0;
    memcpy(&mbx_sym, fuzz_data + (sizeof(cavs_vector) * top_len + sizeof(cavs_vector) * top_len), sizeof(mbx_sym));
    
    
    h->mbx = mbx_sym;

    // Width is not directly used by our slice but set for realism
    h->mb_width = 4;

    // Call entry (must directly call vulnerable function)
    entry_func(h);
    return 0;
}
