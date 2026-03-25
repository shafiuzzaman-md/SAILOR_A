#include <string.h>
#include "harness_types.h"
#include <stdlib.h>
// klee removed for replay

// Prototype from harness
void jpeg_idct_4x8(j_decompress_ptr cinfo, jpeg_component_info * compptr,
                   JCOEFPTR coef_block, JSAMPARRAY output_buf, JDIMENSION output_col);

int main() {
    // Allocate cinfo (not used by our slice beyond passing to IDCT_range_limit)
    struct jpeg_decompress_struct *cinfo_obj = (struct jpeg_decompress_struct *)calloc(1, sizeof(struct jpeg_decompress_struct));
    j_decompress_ptr cinfo = (j_decompress_ptr)cinfo_obj;

    // Allocate component info and its dct_table to satisfy potential preconditions
    jpeg_component_info *compptr = (jpeg_component_info *)calloc(1, sizeof(jpeg_component_info));
    if (compptr) {
        compptr->dct_table = calloc(64, sizeof(ISLOW_MULT_TYPE));
    }

    // Allocate coef_block (unused in our slice but needed for signature)
    JCOEFPTR coef_block = (JCOEFPTR)calloc(64, sizeof(JCOEF));

    // Prepare output buffer with a TOO-SMALL row (3 bytes) so outptr[3] is OOB
    JSAMPARRAY output_buf = (JSAMPARRAY)calloc(1, sizeof(JSAMPROW));
    JSAMPROW row0 = (JSAMPROW)malloc(3);
    { static const unsigned char row0_bytes_data[] = {0x00, 0x00, 0x00}; memcpy(row0, row0_bytes_data, (3 < sizeof(row0_bytes_data)) ? 3 : sizeof(row0_bytes_data)); };
    output_buf[0] = row0;

    JDIMENSION output_col = 0; // start at beginning of row

    jpeg_idct_4x8(cinfo, compptr, coef_block, output_buf, output_col);
    return 0;
}
