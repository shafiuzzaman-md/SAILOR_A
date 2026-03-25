#include <string.h>
#include "harness_types.h"
#include <stdlib.h>
// klee removed for replay

int main() {
    // cinfo is unused by our neutralized function (IDCT_range_limit ignores it)
    j_decompress_ptr cinfo = 0;

    // compptr and coef_block are unused; provide minimal allocations
    jpeg_component_info *compptr = (jpeg_component_info *)calloc(1, sizeof(*compptr));
    JCOEFPTR coef_block = 0;

    // Prepare output buffer with a single row that is TOO SMALL (size 4)
    JSAMPARRAY output_buf = (JSAMPARRAY)calloc(1, sizeof(JSAMPROW));
    JSAMPROW row = (JSAMPROW)malloc(4); // only 4 bytes; outptr[4] will be OOB
    { static const unsigned char row_bytes_data[] = {0x00, 0x00, 0x00, 0x00}; memcpy(row, row_bytes_data, (4 < sizeof(row_bytes_data)) ? 4 : sizeof(row_bytes_data)); };
    output_buf[0] = row;

    // Constrain output_col to 0 to ensure outptr indexes [0] and [4] map directly into row
    JDIMENSION output_col = 0;

    jpeg_idct_5x5(cinfo, compptr, coef_block, output_buf, output_col);
    return 0;
}
