#include <stddef.h>
#include "harness_types.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
// klee removed for replay

int jbig2_table(Jbig2Ctx *ctx, Jbig2Segment *segment, const byte *segment_data);

int LLVMFuzzerTestOneInput(const uint8_t *fuzz_data, size_t fuzz_size) {
    if (fuzz_size < 64) return 0;
    Jbig2Ctx *ctx = (Jbig2Ctx *)calloc(1, sizeof(Jbig2Ctx));
    Jbig2Segment *segment = (Jbig2Segment *)calloc(1, sizeof(Jbig2Segment));

    const size_t seg_size = 64; // concrete size > 10 and > LOG_TABLE_SIZE_MAX
    byte *seg_data = (byte *)malloc(seg_size);
    if (!ctx || !segment || !seg_data) return 0;

    { memcpy(seg_data, fuzz_data + 0, 64); };

    segment->result = NULL;
    segment->data_length = seg_size; // >= 10
    segment->number = 0;

    jbig2_table(ctx, segment, seg_data);
    return 0;
}
