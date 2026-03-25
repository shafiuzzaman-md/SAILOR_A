#include <stddef.h>
#include <string.h>
// NO_HARNESS_TYPES
// klee removed for replay
#include <stdint.h>
#include <stdlib.h>

// Local typedefs matching harness/jidctint.c
typedef unsigned int JDIMENSION;
typedef unsigned char JSAMPLE;
typedef JSAMPLE *JSAMPROW;
typedef JSAMPLE **JSAMPARRAY;
typedef short JCOEF;
typedef JCOEF * JCOEFPTR;
typedef struct jpeg_decompress_struct { int dummy; } jpeg_decompress_struct;
typedef jpeg_decompress_struct * j_decompress_ptr;
typedef short ISLOW_MULT_TYPE;
typedef struct jpeg_component_info { void *dct_table; } jpeg_component_info;

// Prototype must match harness
void jpeg_idct_6x6 (j_decompress_ptr cinfo, jpeg_component_info * compptr,
                    JCOEFPTR coef_block,
                    JSAMPARRAY output_buf, JDIMENSION output_col);

#ifndef COEF_COUNT
#define COEF_COUNT 64
#endif
#ifndef OUTPUT_ROWS
#define OUTPUT_ROWS 6
#endif
#ifndef OUTPUT_COLS
#define OUTPUT_COLS 8
#endif

int LLVMFuzzerTestOneInput(const uint8_t *fuzz_data, size_t fuzz_size) {
    if (fuzz_size < 260) return 0;
    j_decompress_ptr cinfo = (j_decompress_ptr)calloc(1, sizeof(*cinfo));
    if (!cinfo) return 0;
    { memcpy(cinfo, fuzz_data + 0, 4); };

    jpeg_component_info *compptr = (jpeg_component_info*)calloc(1, sizeof(*compptr));
    if (!compptr) return 0;

    ISLOW_MULT_TYPE *dq = (ISLOW_MULT_TYPE*)calloc(COEF_COUNT, sizeof(ISLOW_MULT_TYPE));
    if (!dq) return 0;
    { memcpy(dq, fuzz_data + 4, 128); };
    compptr->dct_table = (void*)dq; // satisfy potential precondition pattern

    JCOEFPTR coef_block = (JCOEFPTR)calloc(COEF_COUNT, sizeof(*coef_block));
    if (!coef_block) return 0;
    { memcpy(coef_block, fuzz_data + 132, 128); };

    JSAMPARRAY output_buf = (JSAMPARRAY)calloc(OUTPUT_ROWS, sizeof(*output_buf));
    if (!output_buf) return 0;
    for (int r = 0; r < OUTPUT_ROWS; r++) {
        output_buf[r] = (JSAMPROW)calloc(OUTPUT_COLS, sizeof(*output_buf[r]));
        if (!output_buf[r]) return 0;
        { static const unsigned char output_row_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(output_buf[r], output_row_data, (OUTPUT_COLS * sizeof(*output_buf[r]) < sizeof(output_row_data)) ? OUTPUT_COLS * sizeof(*output_buf[r]) : sizeof(output_row_data)); };
    }

    JDIMENSION output_col = 0;

    jpeg_idct_6x6(cinfo, compptr, coef_block, output_buf, output_col);
    return 0;
}
