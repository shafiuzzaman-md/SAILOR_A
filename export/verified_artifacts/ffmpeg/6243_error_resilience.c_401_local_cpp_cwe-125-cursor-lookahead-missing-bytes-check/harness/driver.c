// NO_HARNESS_TYPES
#include <klee/klee.h>
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

int main() {
    struct ERContext *s = (struct ERContext *)calloc(1, sizeof(*s));
    if (!s) return 0;

    // Concrete dimensions (small, non-zero)
    s->mb_stride = 4;  // ptrdiff_t
    s->mb_height = 4;  // int

    // Deliberately SMALL buffer to trigger OOB when computing fixed and reading fixed[0]
    size_t buf_sz = 64; // much smaller than required (~384 bytes for 3 blocklists)
    s->er_temp_buffer = (uint8_t *)malloc(buf_sz);
    if (!s->er_temp_buffer) return 0;
    klee_make_symbolic(s->er_temp_buffer, buf_sz, "er_temp_buffer");

    int *decode_error_flags = (int *)calloc(4, sizeof(int));
    if (!decode_error_flags) return 0;
    klee_make_symbolic(decode_error_flags, 4 * sizeof(int), "decode_error_flags");

    entry_func(s, decode_error_flags);
    return 0;
}
