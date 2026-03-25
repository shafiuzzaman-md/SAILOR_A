#include <stddef.h>
// NO_HARNESS_TYPES
// klee removed for replay
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

// Minimal reproduced types matching harness/jidctint.c
typedef unsigned char JSAMPLE;
typedef JSAMPLE *JSAMPROW;
typedef JSAMPROW *JSAMPARRAY;
typedef unsigned int JDIMENSION;
typedef short JCOEF;
typedef JCOEF * JCOEFPTR;

typedef struct jpeg_decompress_struct { int dummy; } * j_decompress_ptr;

typedef struct jpeg_component_info {
    void *dct_table;
} jpeg_component_info;

// Forward declaration matching harness function signature
void jpeg_idct_4x4 (j_decompress_ptr cinfo, jpeg_component_info * compptr,
                    JCOEFPTR coef_block, JSAMPARRAY output_buf, JDIMENSION output_col);

int LLVMFuzzerTestOneInput(const uint8_t *fuzz_data, size_t fuzz_size) {
    if (fuzz_size < 2) return 0;
    // Allocate and initialize cinfo
    struct jpeg_decompress_struct *cinfo = (struct jpeg_decompress_struct *)calloc(1, sizeof(*cinfo));

    // Allocate component info and ensure dct_table is non-NULL as per precondition
    jpeg_component_info *compptr = (jpeg_component_info *)calloc(1, sizeof(*compptr));
    void *fake_table = malloc(16);
    compptr->dct_table = fake_table;  // non-NULL to satisfy quantptr assignment path

    // Allocate coef_block (valid pointer, contents irrelevant in our slice)
    JCOEFPTR coef_block = (JCOEFPTR)calloc(64, sizeof(*coef_block));

    // Prepare output buffer with too-small row to trigger outptr[3] OOB
    JSAMPARRAY output_buf = (JSAMPARRAY)calloc(4, sizeof(*output_buf));

    // Allocate a very small row (2 bytes) so outptr[3] is OOB
    JSAMPROW row0 = (JSAMPROW)malloc(2);
    { memcpy(row0, fuzz_data + 0, 2); };
    output_buf[0] = row0;

    // Other rows unused in our slice
    output_buf[1] = NULL;
    output_buf[2] = NULL;
    output_buf[3] = NULL;

    JDIMENSION output_col = 0; // start at col 0 so outptr == row start

    // Direct call to entry/vulnerable function
    jpeg_idct_4x4(cinfo, compptr, coef_block, output_buf, output_col);

    return 0;
}
