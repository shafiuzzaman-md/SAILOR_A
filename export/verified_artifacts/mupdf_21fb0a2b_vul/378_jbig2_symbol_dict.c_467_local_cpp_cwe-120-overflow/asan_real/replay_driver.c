#include "harness_types.h"
#include <stdlib.h>
#include <string.h>
// klee removed for replay

// Prototypes from harness
extern int jbig2_symbol_dictionary(Jbig2Ctx *ctx, Jbig2Segment *segment, const byte *segment_data);
Jbig2SymbolDictParams g_params;

int main() {
    // Allocate context and segment (minimal; harness doesn't dereference)
    Jbig2Ctx *ctx = (Jbig2Ctx *)calloc(1, sizeof(Jbig2Ctx));
    Jbig2Segment *seg = (Jbig2Segment *)calloc(1, sizeof(Jbig2Segment));

    // Provide some segment data buffer
    byte *segment_data = (byte *)calloc(1, 32);
    { static const unsigned char segment_data_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(segment_data, segment_data_data, (32 < sizeof(segment_data_data)) ? 32 : sizeof(segment_data_data)); };

    // Prepare params: SDTEMPLATE == 0 -> sdat_bytes = 8; provide too-small sdat to cause OOB read in memcpy
    static byte sdat_buf[1];
    { static const unsigned char sdat_buf_data[] = {0x00}; memcpy(sdat_buf, sdat_buf_data, (sizeof(sdat_buf) < sizeof(sdat_buf_data)) ? sizeof(sdat_buf) : sizeof(sdat_buf_data)); };

    g_params.SDTEMPLATE = 0;     // force sdat_bytes = 8
    g_params.sdat = sdat_buf;    // only 1 byte available -> memcpy reads past end

    // Call entry (neutralized passthrough)
    (void) jbig2_symbol_dictionary(ctx, seg, segment_data);

    return 0;
}
