#include <stddef.h>
#include <string.h>
// NO_HARNESS_TYPES
#include <stdlib.h>
#include <stdint.h>
// klee removed for replay

// Minimal local typedefs matching harness/jidctint.c
typedef unsigned int JDIMENSION;
typedef unsigned char JSAMPLE; typedef JSAMPLE *JSAMPROW; typedef JSAMPROW *JSAMPARRAY;
typedef short JCOEF; typedef JCOEF *JCOEFPTR;

typedef struct jpeg_decompress_struct { int dummy; } jpeg_decompress_struct; 
typedef jpeg_decompress_struct * j_decompress_ptr;

typedef struct jpeg_component_info { void *dct_table; } jpeg_component_info;

// Function prototype (matches harness definition)
int jpeg_idct_9x9(j_decompress_ptr cinfo, jpeg_component_info * compptr,
                  JCOEFPTR coef_block, JSAMPARRAY output_buf, JDIMENSION output_col);

int LLVMFuzzerTestOneInput(const uint8_t *fuzz_data, size_t fuzz_size) {
    if (fuzz_size < 128) return 0;
    // Allocate objects for function signature
    j_decompress_ptr cinfo = (j_decompress_ptr)calloc(1, sizeof(struct jpeg_decompress_struct));
    jpeg_component_info *compptr = (jpeg_component_info *)calloc(1, sizeof(jpeg_component_info));

    // Set dct_table to a valid non-NULL pointer per preconditions
    void *tbl = calloc(64, 2); // arbitrary allocation
    compptr->dct_table = tbl;

    // Coef block and output buffer
    JCOEFPTR coef_block = (JCOEFPTR)calloc(64, sizeof(JCOEF));

    // Create a small output buffer
    size_t rows = 4, cols = 16;
    JSAMPARRAY output_buf = (JSAMPARRAY)calloc(rows, sizeof(JSAMPROW));
    for (size_t r = 0; r < rows; ++r) {
        output_buf[r] = (JSAMPROW)calloc(cols, sizeof(JSAMPLE));
        { static const unsigned char output_row_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(output_buf[r], output_row_data, (cols * sizeof(JSAMPLE) < sizeof(output_row_data)) ? cols * sizeof(JSAMPLE) : sizeof(output_row_data)); };
    }

    if (coef_block) { memcpy(coef_block, fuzz_data + 0, 128); };

    JDIMENSION output_col;
    { static const unsigned char output_col_data[] = {0x00, 0x00, 0x00, 0x00}; memcpy(&output_col, output_col_data, (sizeof(output_col) < sizeof(output_col_data)) ? sizeof(output_col) : sizeof(output_col_data)); };

    // Direct call to entry/vulnerable function
    jpeg_idct_9x9(cinfo, compptr, coef_block, output_buf, output_col);

    return 0;
}
