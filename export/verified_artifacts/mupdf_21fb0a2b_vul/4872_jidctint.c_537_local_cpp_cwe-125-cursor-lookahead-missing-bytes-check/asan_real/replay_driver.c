#include <string.h>
// NO_HARNESS_TYPES
#include <stdlib.h>
// klee removed for replay

// Minimal local type defs (must match harness/jidctint.c)
typedef unsigned int JDIMENSION;
typedef unsigned char JSAMPLE;
typedef JSAMPLE * JSAMPROW;
typedef JSAMPLE ** JSAMPARRAY;
typedef short JCOEF;
typedef JCOEF * JCOEFPTR;

typedef struct jpeg_decompress_struct { int dummy; } jpeg_decompress_struct;
typedef jpeg_decompress_struct * j_decompress_ptr;

typedef struct jpeg_component_info { void *dct_table; } jpeg_component_info;

void jpeg_idct_7x7 (j_decompress_ptr cinfo, jpeg_component_info * compptr,
                    JCOEFPTR coef_block, JSAMPARRAY output_buf, JDIMENSION output_col);

int main() {
    j_decompress_ptr cinfo = (j_decompress_ptr)calloc(1, sizeof(struct jpeg_decompress_struct));
    jpeg_component_info *compptr = (jpeg_component_info *)calloc(1, sizeof(struct jpeg_component_info));

    void *qtbl = malloc(64);
    compptr->dct_table = qtbl ? qtbl : compptr; // ensure non-NULL per preconditions

    // Allocate only 2 ints; vulnerable code reads wsptr[2]
    void *raw = malloc(2 * sizeof(int));
    if (!raw) return 0;
    { static const unsigned char coef_raw_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(raw, coef_raw_data, (2 * sizeof(int) < sizeof(coef_raw_data)) ? 2 * sizeof(int) : sizeof(coef_raw_data)); };

    JCOEFPTR coef_block = (JCOEFPTR)raw;
    JSAMPARRAY output_buf = 0;
    JDIMENSION output_col = 0;

    jpeg_idct_7x7(cinfo, compptr, coef_block, output_buf, output_col);
    return 0;
}
