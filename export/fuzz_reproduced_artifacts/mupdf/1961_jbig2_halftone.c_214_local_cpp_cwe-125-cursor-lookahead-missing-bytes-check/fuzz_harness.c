#include <stddef.h>
#include <string.h>
#include "harness_types.h"
// klee removed for replay
#include <stdint.h>
#include <stdlib.h>

int jbig2_pattern_dictionary(Jbig2Ctx *ctx, Jbig2Segment *segment, const byte *segment_data);

int LLVMFuzzerTestOneInput(const uint8_t *fuzz_data, size_t fuzz_size) {
    if (fuzz_size < 2) return 0;
    Jbig2Ctx *ctx = (Jbig2Ctx *)calloc(1, sizeof(Jbig2Ctx));
    Jbig2Segment *seg = (Jbig2Segment *)calloc(1, sizeof(Jbig2Segment));

    // Satisfy entry preconditions to avoid early return
    seg->data_length = 7; // concrete, >=7
    seg->number = 0;

    // Under-sized buffer to trigger OOB read at segment_data[2]
    const size_t buf_sz = 2; // < 3
    byte *data = (byte *)malloc(buf_sz);
    { memcpy(data, fuzz_data + 0, 2); };

    // Direct call to entry
    jbig2_pattern_dictionary(ctx, seg, data);
    return 0;
}
