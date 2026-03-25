#include <stddef.h>
#include <string.h>
#include "harness_types.h"
// klee removed for replay
#include <stdlib.h>
#include <stdint.h>

/* Prototype from harness */
void jpeg_idct_2x2 (j_decompress_ptr cinfo, struct jpeg_component_info * compptr,
               JCOEFPTR coef_block,
               JSAMPARRAY output_buf, JDIMENSION output_col);

int LLVMFuzzerTestOneInput(const uint8_t *fuzz_data, size_t fuzz_size) {
    if (fuzz_size < 136) return 0;
    // Allocate opaque memory for cinfo and compptr (types may be incomplete here)
    void *cinfo_mem = calloc(1, 64);
    j_decompress_ptr cinfo = (j_decompress_ptr)cinfo_mem;

    void *comp_mem = calloc(1, 64);
    struct jpeg_component_info *comp = (struct jpeg_component_info*)comp_mem;

    // Allocate coefficient block and make symbolic (not used by sliced body)
    JCOEFPTR block = (JCOEFPTR)calloc(DCTSIZE*DCTSIZE, sizeof(JCOEF));
    { memcpy(block, fuzz_data + 0, 128); };

    // Prepare output buffer: one row pointer only (our slice uses row 0)
    const size_t ROW_SIZE = 8; // intentionally small to encourage OOB
    JSAMPROW row0 = (JSAMPROW)malloc(ROW_SIZE);
    { memcpy(row0, fuzz_data + 128, 8); };

    JSAMPARRAY out = (JSAMPARRAY)malloc(sizeof(JSAMPROW) * 1);
    out[0] = row0;

    // Symbolic output_col forced OUT-OF-BOUNDS to trigger ptr.err on outptr[0]
    JDIMENSION output_col;
    { static const unsigned char output_col_data[] = {0x08, 0x00, 0x00, 0x00}; memcpy(&output_col, output_col_data, (sizeof(output_col) < sizeof(output_col_data)) ? sizeof(output_col) : sizeof(output_col_data)); };
    /* klee_assume removed */
    /* klee_assume removed */ // reasonable upper bound

    // Directly call the vulnerable/entry function
    jpeg_idct_2x2(cinfo, comp, block, out, output_col);

    return 0;
}
