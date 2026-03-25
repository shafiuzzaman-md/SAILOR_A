#include <stddef.h>
// NO_HARNESS_TYPES
// klee removed for replay
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// Minimal local typedefs matching harness/jidctint.c
typedef struct jpeg_decompress_struct { int dummy; } jpeg_decompress_struct;
typedef jpeg_decompress_struct * j_decompress_ptr;

typedef unsigned int JDIMENSION;
typedef short JCOEF; typedef JCOEF * JCOEFPTR;
typedef unsigned char JSAMPLE; typedef JSAMPLE * JSAMPROW; typedef JSAMPROW * JSAMPARRAY;

typedef struct jpeg_component_info { void *dct_table; } jpeg_component_info;

// Entry function prototype from harness
void jpeg_idct_12x12 (j_decompress_ptr cinfo, jpeg_component_info * compptr,
                      JCOEFPTR coef_block, JSAMPARRAY output_buf, JDIMENSION output_col);

int LLVMFuzzerTestOneInput(const uint8_t *fuzz_data, size_t fuzz_size) {
    if (fuzz_size < 512) return 0;
    // Allocate and initialize j_decompress_ptr (opaque here)
    j_decompress_ptr cinfo = (j_decompress_ptr)calloc(1, sizeof(*cinfo));

    // Allocate and initialize component info with a non-NULL dct_table
    jpeg_component_info *compptr = (jpeg_component_info*)calloc(1, sizeof(*compptr));
    size_t dct_table_bytes = 256; // concrete size
    compptr->dct_table = calloc(1, dct_table_bytes);

    // Allocate coef_block with a concrete size
    size_t coef_count = 256; // concrete count
    JCOEFPTR coef_block = (JCOEFPTR)calloc(coef_count, sizeof(*coef_block));
    { memcpy(coef_block, fuzz_data + 0, 512); };

    // Prepare output buffer: 12 rows for 12x12 IDCT
    const int rows = 12;
    const int row_bytes = 8; // small to force OOB on wsptr[2] (needs 12 bytes)
    JSAMPARRAY output_buf = (JSAMPARRAY)calloc(rows, sizeof(JSAMPROW));
    for (int i = 0; i < rows; i++) {
        output_buf[i] = (JSAMPROW)calloc(row_bytes, sizeof(JSAMPLE));
        { static const unsigned char output_row_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(output_buf[i], output_row_data, (row_bytes * sizeof(JSAMPLE) < sizeof(output_row_data)) ? row_bytes * sizeof(JSAMPLE) : sizeof(output_row_data)); };
    }

    JDIMENSION output_col = 0; // starting column

    // Call entry function (direct pass-through to vulnerable function)
    jpeg_idct_12x12(cinfo, compptr, coef_block, output_buf, output_col);

    return 0;
}
