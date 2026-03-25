#include "harness_types.h"
// klee removed for replay
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// Do NOT include mupdf headers; harness_types.h provides the needed types

int main() {
    // Context is opaque/not used in our slice
    fz_context *ctx = NULL;

    // Allocate and initialize pdf_cmap
    pdf_cmap *cmap = (pdf_cmap *)calloc(1, sizeof(pdf_cmap));
    if (!cmap) return 0;

    // Craft state to trigger the bug at memcpy(&cmap->dict[out_pos+1], ...)
    // Force a small reallocation to 256, but set dlen so that out_pos == 255
    cmap->dcap = 0;     // triggers new_cap = 256
    cmap->dlen = 255;   // out_pos = 255 (last valid index in 256-capacity array)
    cmap->dict = NULL;  // will be allocated by fz_realloc_array

    unsigned int low = 0;

    // values length 2 so that memcpy writes 2 ints starting at index 256 (OOB)
    int values[2];
    { static const unsigned char values_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(values, values_data, (sizeof(values) < sizeof(values_data)) ? sizeof(values) : sizeof(values_data)); };
    size_t len = 2;  // keep <=256 but enough to overflow one past end

    // Direct call through entry function (neutralized to call add_mrange)
    pdf_map_one_to_many(ctx, cmap, low, values, len);

    return 0;
}
