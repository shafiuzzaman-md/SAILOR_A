#include <string.h>
#include "harness_types.h"
// klee removed for replay
#include <stdlib.h>
#include <stdint.h>

// Prototypes from harness
int jbig2_data_in(Jbig2Ctx *ctx, const unsigned char *data, size_t size);

int main() {
    // Allocate context (layout provided by harness_types.h)
    Jbig2Ctx *ctx = (Jbig2Ctx *)calloc(1, sizeof(Jbig2Ctx));

    // Allocate a small concrete buffer (3 bytes) to trigger OOB read in jbig2_get_uint32
    const size_t data_size = 3; // < 4 bytes so get_uint16(bptr+2) reads past end
    unsigned char *data = (unsigned char *)malloc(data_size);

    // Make contents symbolic so KLEE explores values
    { static const unsigned char data_bytes_data[] = {0x00, 0x00, 0x00}; memcpy(data, data_bytes_data, (data_size < sizeof(data_bytes_data)) ? data_size : sizeof(data_bytes_data)); };

    // Directly call entry; harness neutralization passes through to vulnerable function
    (void)jbig2_data_in(ctx, data, data_size);

    return 0;
}
