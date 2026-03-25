#include <string.h>
// NO_HARNESS_TYPES
#include <stdlib.h>
// klee removed for replay

/* Minimal local type shims matching harness/jidctint.c */
typedef struct jpeg_decompress_struct * j_decompress_ptr;

typedef struct jpeg_component_info {
    void *dct_table;
} jpeg_component_info;

typedef short JCOEF; typedef JCOEF * JCOEFPTR;
typedef unsigned char JSAMPLE; typedef JSAMPLE * JSAMPROW; typedef JSAMPROW * JSAMPARRAY;
typedef unsigned int JDIMENSION;

/* Prototype from harness */
void jpeg_idct_10x5(j_decompress_ptr cinfo, jpeg_component_info * compptr,
                    JCOEFPTR coef_block, JSAMPARRAY output_buf, JDIMENSION output_col);

int main() {
    j_decompress_ptr cinfo = NULL; // not used by harness path

    jpeg_component_info *compptr = (jpeg_component_info *)calloc(1, sizeof(*compptr));
    int dummy_table = 0; // satisfy precondition quantptr = (ISLOW_MULT_TYPE*) compptr->dct_table
    compptr->dct_table = &dummy_table;

    JCOEFPTR coef_block = (JCOEFPTR)calloc(64, sizeof(*coef_block));

    size_t rows = 5;
    JSAMPARRAY output_buf = (JSAMPARRAY)calloc(rows, sizeof(*output_buf));
    for (size_t r = 0; r < rows; r++) {
        output_buf[r] = (JSAMPROW)calloc(16, sizeof(*output_buf[r]));
    }

    JDIMENSION output_col = 0;

    jpeg_idct_10x5(cinfo, compptr, coef_block, output_buf, output_col);
    return 0;
}
