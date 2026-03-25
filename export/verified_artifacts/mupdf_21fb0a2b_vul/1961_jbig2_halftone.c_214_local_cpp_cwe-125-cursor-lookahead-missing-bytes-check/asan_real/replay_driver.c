#include <string.h>
#include "harness_types.h"
// klee removed for replay
#include <stdint.h>
#include <stdlib.h>

int jbig2_pattern_dictionary(Jbig2Ctx *ctx, Jbig2Segment *segment, const byte *segment_data);

int main() {
    Jbig2Ctx *ctx = (Jbig2Ctx *)calloc(1, sizeof(Jbig2Ctx));
    Jbig2Segment *seg = (Jbig2Segment *)calloc(1, sizeof(Jbig2Segment));

    // Satisfy entry preconditions to avoid early return
    seg->data_length = 7; // concrete, >=7
    seg->number = 0;

    // Under-sized buffer to trigger OOB read at segment_data[2]
    const size_t buf_sz = 2; // < 3
    byte *data = (byte *)malloc(buf_sz);
    { static const unsigned char segment_data_data[] = {0x00, 0x00}; memcpy(data, segment_data_data, (buf_sz < sizeof(segment_data_data)) ? buf_sz : sizeof(segment_data_data)); };

    // Direct call to entry
    jbig2_pattern_dictionary(ctx, seg, data);
    return 0;
}
