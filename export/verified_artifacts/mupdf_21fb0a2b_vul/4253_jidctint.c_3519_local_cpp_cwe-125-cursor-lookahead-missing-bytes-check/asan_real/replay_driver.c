#include "harness_types.h"
// klee removed for replay
#include <stdlib.h>
#include <string.h>

int main(void) {
    // Allocate and initialize cinfo (not dereferenced by our sliced harness)
    struct jpeg_decompress_struct *cinfo_obj = (struct jpeg_decompress_struct*)calloc(1, sizeof(struct jpeg_decompress_struct));
    j_decompress_ptr cinfo = cinfo_obj;

    // Allocate component info and its dct_table to satisfy preconditions
    struct jpeg_component_info *compptr = (struct jpeg_component_info*)calloc(1, sizeof(struct jpeg_component_info));
    compptr->dct_table = calloc(1, 64); // non-NULL table pointer

    // Allocate coefficient block (kept non-NULL)
    JCOEFPTR coef_block = (JCOEFPTR)calloc(64, sizeof(*coef_block));

    // Prepare output buffer: 1 row pointer
    JSAMPARRAY output_buf = (JSAMPARRAY)calloc(1, sizeof(*output_buf));

    // Large enough row so outptr[11] and following stores are in-bounds
    const size_t row_size = 32; // >= 12
    JSAMPROW row0 = (JSAMPROW)malloc(row_size);
    { static const unsigned char row0_bytes_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(row0, row0_bytes_data, (row_size < sizeof(row0_bytes_data)) ? row_size : sizeof(row0_bytes_data)); };
    output_buf[0] = row0;

    // Start at column 0 so outptr == row0
    JDIMENSION output_col = 0;

    // Call entry (which is also the vulnerable function in this slice)
    jpeg_idct_12x6(cinfo, compptr, coef_block, output_buf, output_col);

    return 0;
}
