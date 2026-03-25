// Combined reproducer for 12078_vc1_pred.c_550_local_cpp_cwe-125-cursor-lookahead-missing-bytes-check
// Original harness: driver.c + smart_stubs.c + sliced source

// === smart_stubs.c ===
/* Smart stubs — auto-generated from path + vulnerability analysis */
/* Symbolic stubs model the environment: KLEE explores return values */
/* that both REACH the sink AND TRIGGER the vulnerability */
#include <stdlib.h>
#include <string.h>
/* PROACTIVE: function (auto-detected external) */
int function() { return 0; }

/* PROACTIVE: statement (auto-detected external) */
int statement() { return 0; }

// === driver.c ===
// NO_HARNESS_TYPES
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
/* Mirror minimal types used in harness/vc1_pred.c */
typedef struct MpegEncContext {
    int *block_index;
    int mb_x, mb_stride, mb_width, first_slice_line;
} MpegEncContext;

typedef struct VC1Context {
    MpegEncContext s;
} VC1Context;

/* entry_func is defined in harness/vc1_pred.c */
int entry_func(VC1Context *v);

int LLVMFuzzerTestOneInput(const uint8_t *fuzz_data, size_t fuzz_size) {
    if (fuzz_size < 64) return 0;
    VC1Context *v = (VC1Context *)calloc(1, sizeof(VC1Context));
    if (!v) return 0;

    // Allocate too-small block_index (size 2) so access to [2] is OOB
    int *bi = (int *)malloc(2 * sizeof(int));
    if (!bi) return 0;
    memcpy(bi, fuzz_data + (0), 2 * sizeof(int));

    v->s.block_index = bi;
    v->s.mb_x = 0;
    v->s.mb_stride = 0;
    v->s.mb_width = 0;
    v->s.first_slice_line = 0;

    entry_func(v);
    return 0;
}
