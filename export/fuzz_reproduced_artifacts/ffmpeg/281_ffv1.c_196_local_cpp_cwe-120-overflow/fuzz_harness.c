#include <stddef.h>
// Combined reproducer for 281_ffv1.c_196_local_cpp_cwe-120-overflow
// Original harness: driver.c + smart_stubs.c + sliced source

// === smart_stubs.c ===
/* Smart stubs — auto-generated from path + vulnerability analysis */
/* Symbolic stubs model the environment: KLEE explores return values */
/* that both REACH the sink AND TRIGGER the vulnerability */
#include <stdlib.h>
#include <string.h>
/* PROACTIVE: FUNCTION (auto-detected external) */
int FUNCTION() { return 0; }

// === driver.c ===
#include "harness_types.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifndef CONTEXT_SIZE
#define CONTEXT_SIZE 32
#endif
#ifndef MAX_QUANT_TABLES
#define MAX_QUANT_TABLES 8
#endif

// Entry prototype from harness
int ffv1_entry(FFV1Context *f, FFV1SliceContext *sc);

int LLVMFuzzerTestOneInput(const uint8_t *fuzz_data, size_t fuzz_size) {
    if (fuzz_size < 64) return 0;
    // Allocate contexts
    FFV1Context *f = (FFV1Context *)calloc(1, sizeof(FFV1Context));
    FFV1SliceContext *sc = (FFV1SliceContext *)calloc(1, sizeof(FFV1SliceContext));

    // Set minimal valid configuration
    f->plane_count = 1;
    f->ac = 1; // ensure f->ac != AC_GOLOMB_RICE (0) to take memcpy path

    // Allocate plane array
    sc->plane = (PlaneContext *)calloc(f->plane_count, sizeof(PlaneContext));

    // Configure plane 0
    PlaneContext *p = &sc->plane[0];
    p->quant_table_index = 0;

    // Choose a context_count so the copy size is larger than dest to trigger overflow
    p->context_count = 2; // copy size = CONTEXT_SIZE * 2 = 64 bytes

    // Source initial_states[0] must be non-NULL and large enough
    size_t src_len = (size_t)CONTEXT_SIZE * (size_t)p->context_count; // 64
    uint8_t *src = (uint8_t *)malloc(src_len);
    // Make source content symbolic (not size)
    memcpy(src, fuzz_data + (0), src_len);
    f->initial_states[0] = src;

    // Destination state: intentionally under-allocate to cause overflow
    size_t dst_len = 16; // smaller than src_len
    p->state = (uint8_t *)malloc(dst_len);
    memcpy(p->state, fuzz_data + (src_len), dst_len);

    // Call entry which directly calls the vulnerable function
    ffv1_entry(f, sc);

    return 0;
}
