#include <string.h>
#include "harness_types.h"
// klee removed for replay
#include <stdlib.h>
#include <stdint.h>

// Entry/vulnerable function
extern void jpeg_idct_8x4(j_decompress_ptr cinfo, jpeg_component_info * compptr,
                          JCOEFPTR coef_block, JSAMPARRAY output_buf, JDIMENSION output_col);

int main() {
    // 1) Concrete allocations for required arguments
    jpeg_decompress_struct *cinfo = (jpeg_decompress_struct*)calloc(1, sizeof(jpeg_decompress_struct));
    jpeg_component_info *compptr = (jpeg_component_info*)calloc(1, sizeof(jpeg_component_info));

    // dct_table is not used in the sliced body, but allocate to satisfy any potential preconditions
    compptr->dct_table = calloc(64, sizeof(ISLOW_MULT_TYPE));

    // coef_block (not used by sliced body, but allocate realistically)
    JCOEFPTR coef_block = (JCOEFPTR)calloc(64, sizeof(JCOEF));

    // output buffer: 4 rows; each row a small buffer to provoke OOB on outptr[1]
    const int rows = 4;
    const int row_len = 1; // only 1 byte so indexing [1] is OOB
    JSAMPARRAY output_buf = (JSAMPARRAY)calloc(rows, sizeof(JSAMPROW));
    for (int i = 0; i < rows; ++i) {
        output_buf[i] = (JSAMPROW)malloc(row_len);
        // Make row contents symbolic (size is concrete and correct)
        { static const unsigned char row_bytes_data[] = {0x00}; memcpy(output_buf[i], row_bytes_data, (row_len < sizeof(row_bytes_data)) ? row_len : sizeof(row_bytes_data)); };
    }

    // Make coef_block contents symbolic too (size is concrete)
    { static const unsigned char coef_block_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(coef_block, coef_block_data, (64 * sizeof(JCOEF) < sizeof(coef_block_data)) ? 64 * sizeof(JCOEF) : sizeof(coef_block_data)); };

    // Choose output_col so that outptr = output_buf[0] + output_col; keep 0 for simpler OOB at [1]
    JDIMENSION output_col = 0;

    // 2) Call entry (same as vulnerable function in this case)
    jpeg_idct_8x4(cinfo, compptr, coef_block, output_buf, output_col);

    return 0;
}
