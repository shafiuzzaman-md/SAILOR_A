#include <stdint.h>
#include <stddef.h>
#include <klee/klee.h>
#include <stdatomic.h>

// Local minimal ERContext matching the fields used in this slice
struct ERContext {
    void *avctx;                 // AVCodecContext * (opaque)
    void *sad;                   // me_cmp_func (opaque)
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

// Use the auto-injected harness_types.h for real project types (ERContext, etc.)
// We rely on fields present in the provided struct definitions:
//   - ptrdiff_t mb_stride;
//   - int mb_height;
//   - uint8_t *er_temp_buffer;
// Other fields are unused in this slice.

// Entry: simple pass-through to the vulnerable function (no guards!)
int entry_func(struct ERContext *s, int *decode_error_flags);
void ff_er_frame_end(struct ERContext *s, int *decode_error_flags);

int entry_func(struct ERContext *s, int *decode_error_flags) {
    ff_er_frame_end(s, decode_error_flags); // DIRECT call, no checks
    return 0;
}

void ff_er_frame_end(struct ERContext *s, int *decode_error_flags) {
    // Minimal neutralized slice focusing on the vulnerable area around line ~401
    int (*blocklist)[2];
    int (*next_blocklist)[2];
    int (*tmp_blocklist)[2];
    uint8_t *fixed;
    int32_t *pred_count;

    // These computations mirror the layout from error_resilience.c around the target site
    blocklist      = (int (*)[2])s->er_temp_buffer;
    next_blocklist = blocklist + s->mb_stride * s->mb_height;
    tmp_blocklist  = next_blocklist + s->mb_stride * s->mb_height;
    (void)tmp_blocklist; // not used further in this slice

    fixed          = (uint8_t *)(next_blocklist + s->mb_stride * s->mb_height);

    // Materialize the access that occurs later in the real function using the computed buffer
    // This read will go OOB if er_temp_buffer is too small for the required blocklists
    uint8_t byte0 = fixed[0];
    (void)byte0;

    // UNIVERSAL SINK ASSERTION — after the vulnerable statement
    klee_assert(0 && "SAILOR_SINK_REACHED");
}
