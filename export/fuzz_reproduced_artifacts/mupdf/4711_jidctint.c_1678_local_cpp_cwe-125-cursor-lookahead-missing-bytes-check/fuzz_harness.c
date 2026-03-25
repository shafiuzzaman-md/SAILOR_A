#include <stdint.h>
#include <stddef.h>
#include <string.h>
// NO_HARNESS_TYPES
#include <stdlib.h>
// klee removed for replay

// Minimal type defs matching harness/jidctint.c
typedef unsigned char JSAMPLE;
typedef JSAMPLE * JSAMPROW;
typedef JSAMPROW * JSAMPARRAY;
typedef unsigned int JDIMENSION;
typedef int ISLOW_MULT_TYPE;
typedef short * JCOEFPTR;

typedef struct jpeg_decompress_struct {
    JSAMPLE *sample_range_limit;
} jpeg_decompress_struct;

typedef jpeg_decompress_struct * j_decompress_ptr;

typedef struct jpeg_component_info {
    void *dct_table;
} jpeg_component_info;

// Prototype must match harness exactly
void jpeg_idct_11x11 (j_decompress_ptr cinfo, jpeg_component_info * compptr,
                      JCOEFPTR coef_block,
                      JSAMPARRAY output_buf, JDIMENSION output_col);

int LLVMFuzzerTestOneInput(const uint8_t *fuzz_data, size_t fuzz_size) {
    if (fuzz_size < 393) return 0;
    // Decompressor context and range_limit buffer
    jpeg_decompress_struct *cinfo = (jpeg_decompress_struct *)calloc(1, sizeof(jpeg_decompress_struct));
    JSAMPLE *range = (JSAMPLE *)malloc(1);
    { memcpy(range, fuzz_data + 0, 1); };
    cinfo->sample_range_limit = range;

    // Component info and its dct_table per entry precondition
    jpeg_component_info *comp = (jpeg_component_info *)calloc(1, sizeof(jpeg_component_info));
    ISLOW_MULT_TYPE *dct = (ISLOW_MULT_TYPE *)malloc(sizeof(ISLOW_MULT_TYPE) * 64);
    { memcpy(dct, fuzz_data + 1, 256); };
    comp->dct_table = (void *)dct;

    // Coefficient block (valid pointer, content symbolic)
    JCOEFPTR coef = (JCOEFPTR)calloc(64, sizeof(*coef));
    { memcpy(coef, fuzz_data + 257, 128); };

    // Output buffer: only 8 bytes to trigger OOB at outptr[8]
    JSAMPROW row0 = (JSAMPROW)malloc(8);
    { memcpy(row0, fuzz_data + 385, 8); };

    JSAMPARRAY out = (JSAMPARRAY)malloc(sizeof(JSAMPROW));
    out[0] = row0;

    JDIMENSION output_col = 0;

    // Direct call to entry/vulnerable function
    jpeg_idct_11x11(cinfo, comp, coef, out, output_col);
    return 0;
}
