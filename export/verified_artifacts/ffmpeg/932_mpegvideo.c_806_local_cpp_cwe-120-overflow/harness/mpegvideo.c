#include <stdint.h>
#include <string.h>
#include <klee/klee.h>

// Minimal local reproduction of the needed context type
// Keep only fields used on the direct path
typedef struct MpegEncContext {
    int b8_stride;
    int block_index[6];
    // ac_val: per-plane arrays of 16-int16_t blocks
    int16_t (*ac_val[3])[16];
} MpegEncContext;

// Vulnerable function (neutralized to the minimal path)
void ff_clean_intra_table_entries(MpegEncContext *s)
{
    int wrap = s->b8_stride;
    int xy = s->block_index[0];

    /* ac pred */
    memset(s->ac_val[0][xy       ], 0, 32 * sizeof(int16_t));
    // UNIVERSAL SINK ASSERTION — after the vulnerable statement
    klee_assert(0 && "SAILOR_SINK_REACHED");

    // Minimal: stop here — remaining original body removed
}

// Entry function — mandatory simple pass-through
int harness_entry(MpegEncContext *s) {
    ff_clean_intra_table_entries(s);
    return 0;
}
