#include <stddef.h>
#include <string.h>
// NO_HARNESS_TYPES
#include <stdlib.h>
// klee removed for replay
#include <stdint.h>

typedef unsigned int JDIMENSION;
typedef int INT32;
typedef int ISLOW_MULT_TYPE;
typedef unsigned char JSAMPLE;
typedef JSAMPLE * JSAMPROW;
typedef JSAMPROW * JSAMPARRAY;
typedef short JCOEF;
typedef JCOEF * JCOEFPTR;

struct jpeg_decompress_struct { int dummy; };
typedef struct jpeg_decompress_struct * j_decompress_ptr;

typedef struct jpeg_component_info {
    void *dct_table;
} jpeg_component_info;

void jpeg_idct_16x16 (j_decompress_ptr cinfo, jpeg_component_info * compptr,
                      JCOEFPTR coef_block,
                      JSAMPARRAY output_buf, JDIMENSION output_col);

int LLVMFuzzerTestOneInput(const uint8_t *fuzz_data, size_t fuzz_size) {
    if (fuzz_size < 1024) return 0;
    j_decompress_ptr cinfo = (j_decompress_ptr)calloc(1, sizeof(struct jpeg_decompress_struct));
    jpeg_component_info *compptr = (jpeg_component_info *)calloc(1, sizeof(jpeg_component_info));

    ISLOW_MULT_TYPE *qt = (ISLOW_MULT_TYPE *)calloc(256, sizeof(ISLOW_MULT_TYPE));
    { memcpy(qt, fuzz_data + 0, 512); };
    compptr->dct_table = qt;

    JCOEFPTR coef_block = (JCOEFPTR)calloc(256, sizeof(JCOEF));
    { memcpy(coef_block, fuzz_data + 512, 512); };

    const int rows = 16, cols = 16;
    JSAMPARRAY output_buf = (JSAMPARRAY)calloc(rows, sizeof(JSAMPROW));
    for (int i = 0; i < rows; i++) {
        output_buf[i] = (JSAMPROW)calloc(cols, sizeof(JSAMPLE));
        { static const unsigned char out_row_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(output_buf[i], out_row_data, (cols * sizeof(JSAMPLE) < sizeof(out_row_data)) ? cols * sizeof(JSAMPLE) : sizeof(out_row_data)); };
    }

    JDIMENSION output_col = 0;

    jpeg_idct_16x16(cinfo, compptr, coef_block, output_buf, output_col);
    return 0;
}
