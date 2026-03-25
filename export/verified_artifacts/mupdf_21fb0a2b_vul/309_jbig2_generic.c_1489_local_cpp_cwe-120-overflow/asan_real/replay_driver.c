#include "harness_types.h"
#include <stdlib.h>
#include <string.h>
// klee removed for replay

typedef unsigned char byte;

int jbig2_immediate_generic_region(Jbig2Ctx *ctx, Jbig2Segment *segment, const byte *segment_data);

int main() {
    // Allocate concrete objects
    Jbig2Ctx *ctx = (Jbig2Ctx *)calloc(1, sizeof(Jbig2Ctx));
    Jbig2Segment *seg = (Jbig2Segment *)calloc(1, sizeof(Jbig2Segment));

    // Concrete buffer for segment_data
    const size_t BUF_SZ = 64;
    byte *segment_data = (byte *)malloc(BUF_SZ);

    // Make fields symbolic
    { static const unsigned char segment_data[] = {0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(seg, segment_data, (sizeof(*seg) < sizeof(segment_data)) ? sizeof(*seg) : sizeof(segment_data)); };
    { static const unsigned char segment_data_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(segment_data, segment_data_data, (BUF_SZ < sizeof(segment_data_data)) ? BUF_SZ : sizeof(segment_data_data)); };

    // Constrain to trigger overflow in memset(GB_stats, 0, stats_size)
    // GB_stats is 16 bytes in harness; require data_length > 16
    /* klee_assume removed */
    /* klee_assume removed */ // reasonable upper bound to avoid path explosion

    // Other fields can be arbitrary
    // Call entry/vulnerable function
    jbig2_immediate_generic_region(ctx, seg, segment_data);

    return 0;
}
