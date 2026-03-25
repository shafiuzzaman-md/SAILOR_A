#include "harness_types.h"
// klee removed for replay
#include <stdlib.h>
#include <string.h>

int main() {
    // cinfo (unused by slice apart from range table)
    jpeg_decompress_struct cinfo_obj; memset(&cinfo_obj, 0, sizeof(cinfo_obj));
    j_decompress_ptr cinfo = &cinfo_obj;

    // compptr: ensure dct_table non-NULL (though slice doesn't use it to guard)
    jpeg_component_info comp; memset(&comp, 0, sizeof(comp));
    ISLOW_MULT_TYPE dummy_table_entry = 0;
    comp.dct_table = &dummy_table_entry;

    // coef_block: space for two INT32s as read by the slice
    void *coef_mem = malloc(8);
    { static const unsigned char coef_mem_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(coef_mem, coef_mem_data, (8 < sizeof(coef_mem_data)) ? 8 : sizeof(coef_mem_data)); };
    JCOEFPTR coef_block = (JCOEFPTR)(void*)coef_mem;

    // Prepare output buffer: 1 row with only 2 bytes so outptr[2] is OOB
    size_t row_size = 2;
    JSAMPROW row = (JSAMPROW)malloc(row_size);
    { static const unsigned char row_bytes_data[] = {0x00, 0x00}; memcpy(row, row_bytes_data, (row_size < sizeof(row_bytes_data)) ? row_size : sizeof(row_bytes_data)); };

    JSAMPARRAY out = (JSAMPARRAY)malloc(sizeof(JSAMPROW));
    out[0] = row;

    JDIMENSION output_col = 0;

    jpeg_idct_9x9(cinfo, &comp, coef_block, out, output_col);
    return 0;
}
