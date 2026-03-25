#include <string.h>
#include "harness_types.h"
#include <stdlib.h>
#include <stdint.h>
// klee removed for replay

int main() {
    // Allocate cinfo (only passed through to IDCT_range_limit stub)
    j_decompress_ptr cinfo = (j_decompress_ptr)calloc(1, sizeof(struct jpeg_decompress_struct));

    // Set up component info with a valid quantization table
    jpeg_component_info comp;
    ISLOW_MULT_TYPE *qt = (ISLOW_MULT_TYPE *)malloc(sizeof(ISLOW_MULT_TYPE) * 8);
    { static const unsigned char quant_table_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(qt, quant_table_data, (sizeof(ISLOW_MULT_TYPE) * 8 < sizeof(quant_table_data)) ? sizeof(ISLOW_MULT_TYPE) * 8 : sizeof(quant_table_data)); };
    comp.dct_table = (void *)qt;

    // Coefficient block: at least one coefficient used (DC)
    JCOEFPTR coef = (JCOEFPTR)malloc(sizeof(JCOEF) * 1);
    { static const unsigned char coef_block_0_data[] = {0x00, 0x00, 0x00, 0x00}; memcpy(coef, coef_block_0_data, (sizeof(JCOEF) * 1 < sizeof(coef_block_0_data)) ? sizeof(JCOEF) * 1 : sizeof(coef_block_0_data)); };

    // Output buffer: single row with small fixed width to allow OOB via output_col
    const size_t row_len = 8; // intentionally small
    JSAMPARRAY out = (JSAMPARRAY)malloc(sizeof(JSAMPROW) * 1);
    JSAMPROW row0 = (JSAMPROW)malloc(row_len * sizeof(JSAMPLE));
    { static const unsigned char output_row0_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(row0, output_row0_data, (row_len * sizeof(JSAMPLE) < sizeof(output_row0_data)) ? row_len * sizeof(JSAMPLE) : sizeof(output_row0_data)); };
    out[0] = row0;

    // output_col symbolic, allow both in-bounds and out-of-bounds
    JDIMENSION output_col;
    { static const unsigned char output_col_data[] = {0x00, 0x00, 0x00, 0x00}; memcpy(&output_col, output_col_data, (sizeof(output_col) < sizeof(output_col_data)) ? sizeof(output_col) : sizeof(output_col_data)); };
    // Keep a reasonable upper bound to avoid path explosion but permit OOB
    /* klee_assume removed */

    // Call entry/vulnerable function directly (pass-through)
    jpeg_idct_1x1(cinfo, &comp, coef, out, output_col);

    return 0;
}
