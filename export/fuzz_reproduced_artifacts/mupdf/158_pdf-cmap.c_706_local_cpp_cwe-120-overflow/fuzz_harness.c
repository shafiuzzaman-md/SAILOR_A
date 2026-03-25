#include <stdint.h>
#include <stddef.h>
#include "harness_types.h"
#include <stdlib.h>
#include <string.h>
// klee removed for replay

int LLVMFuzzerTestOneInput(const uint8_t *fuzz_data, size_t fuzz_size) {
    if (fuzz_size < 512) return 0;
    // Allocate context and cmap
    fz_context *ctx = (fz_context *)calloc(1, sizeof(fz_context));
    pdf_cmap *cmap = (pdf_cmap *)calloc(1, sizeof(pdf_cmap));

    // Initialize cmap to trigger the vulnerable path: dcap=0, dlen=0, dict=NULL
    cmap->dlen = 0;
    cmap->dcap = 0;
    cmap->dict = NULL;

    // Prepare values array with len=256 (edge that triggers overflow in add_mrange)
    const size_t len = 256u;
    int *values = (int *)malloc(sizeof(int) * len);
    { memcpy(values, fuzz_data + 0, 512); };

    // low can be symbolic or concrete; choose symbolic for exploration
    unsigned int low;
    { static const unsigned char low_data[] = {0x00, 0x00, 0x00, 0x00}; memcpy(&low, low_data, (sizeof(low) < sizeof(low_data)) ? sizeof(low) : sizeof(low_data)); };

    // Call entry function (neutralized to direct call to add_mrange)
    pdf_map_one_to_many(ctx, cmap, low, values, len);

    return 0;
}
