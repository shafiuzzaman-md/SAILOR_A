// Combined reproducer for 6243_error_resilience.c_401_local_cpp_cwe-125-cursor-lookahead-missing-bytes-check
// Original harness: driver.c + smart_stubs.c + sliced source

// === smart_stubs.c ===
/* Smart stubs — auto-generated from path + vulnerability analysis */
/* Symbolic stubs model the environment: KLEE explores return values */
/* that both REACH the sink AND TRIGGER the vulnerability */
#include <stdlib.h>
#include <string.h>
/* PROACTIVE: function (auto-detected external) */
int function() { return 0; }

/* PROACTIVE: types (auto-detected external) */
int types() { return 0; }

// === driver.c ===
// NO_HARNESS_TYPES
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <stdatomic.h>

// Minimal compatible ERContext for driver allocation (must match fields used in harness)
struct ERContext {
    void *avctx;
    void *sad;
    int mecc_inited;
    int *mb_index2xy;
    int mb_num;
    int mb_width, mb_height;
    ptrdiff_t mb_stride;
    ptrdiff_t b8_stride;
    atomic_int error_count;
    int error_occurred;
    uint8_t *error_status_table;
    uint8_t *er_temp_buffer;
};

int entry_func(struct ERContext *s, int *decode_error_flags);

int LLVMFuzzerTestOneInput(const uint8_t *fuzz_data, size_t fuzz_size) {
    if (fuzz_size < 64) return 0;
    struct ERContext *s = (struct ERContext *)calloc(1, sizeof(*s));
    if (!s) return 0;

    // Concrete dimensions (small, non-zero)
    s->mb_stride = 4;  // ptrdiff_t
    s->mb_height = 4;  // int

    // Deliberately SMALL buffer to trigger OOB when computing fixed and reading fixed[0]
    size_t buf_sz = 64; // much smaller than required (~384 bytes for 3 blocklists)
    s->er_temp_buffer = (uint8_t *)malloc(buf_sz);
    if (!s->er_temp_buffer) return 0;
    memcpy(s->er_temp_buffer, fuzz_data + (0), buf_sz);

    int *decode_error_flags = (int *)calloc(4, sizeof(int));
    if (!decode_error_flags) return 0;
    memcpy(decode_error_flags, fuzz_data + (buf_sz), 4 * sizeof(int));

    entry_func(s, decode_error_flags);
    return 0;
}
