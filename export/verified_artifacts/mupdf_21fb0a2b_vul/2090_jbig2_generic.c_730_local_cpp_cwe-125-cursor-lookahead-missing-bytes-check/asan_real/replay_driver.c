#include <string.h>
// NO_HARNESS_TYPES
#include <stdint.h>
#include <stdlib.h>
// klee removed for replay

// Opaque type forward declarations matching harness signatures
typedef struct Jbig2Ctx Jbig2Ctx;
typedef struct Jbig2Segment Jbig2Segment;
typedef struct Jbig2GenericRegionParams Jbig2GenericRegionParams;
typedef struct Jbig2ArithState Jbig2ArithState;
typedef struct Jbig2Image Jbig2Image;
typedef struct Jbig2ArithCx Jbig2ArithCx;

// Define the extern used by the harness
uint8_t *g_line2;

// Entry function from the harness
extern int jbig2_decode_generic_region(Jbig2Ctx *ctx,
    Jbig2Segment *segment,
    const Jbig2GenericRegionParams *params,
    Jbig2ArithState *as,
    Jbig2Image *image,
    Jbig2ArithCx *GB_stats);

int main(void) {
    // Allocate a 1-byte buffer and make its content symbolic
    uint8_t *buf = (uint8_t *)malloc(1);
    if (!buf) return 0;
    { static const unsigned char buf_byte_data[] = {0x00}; memcpy(buf, buf_byte_data, (1 < sizeof(buf_byte_data)) ? 1 : sizeof(buf_byte_data)); };

    // Symbolic offset: 0 (in-bounds) or 1 (OOB when reading line2[0])
    unsigned off;
    { static const unsigned char off_data[] = {0x00, 0x00, 0x00, 0x00}; memcpy(&off, off_data, (sizeof(off) < sizeof(off_data)) ? sizeof(off) : sizeof(off_data)); };
    /* klee_assume removed */
    g_line2 = buf + off;

    // Call entry; harness routes directly to the vulnerable function
    jbig2_decode_generic_region(NULL, NULL, NULL, NULL, NULL, NULL);
    return 0;
}
