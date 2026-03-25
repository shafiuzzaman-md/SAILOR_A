#include <string.h>
// NO_HARNESS_TYPES
#include <stdlib.h>
#include <stdint.h>

// Match the minimal typedefs used in harness/jidctint.c
typedef void * j_decompress_ptr;
typedef unsigned int JDIMENSION;
typedef short JCOEF; typedef JCOEF * JCOEFPTR;
typedef unsigned char JSAMPLE; typedef JSAMPLE * JSAMPROW; typedef JSAMPROW * JSAMPARRAY;

typedef struct jpeg_component_info { void * dct_table; } jpeg_component_info;

// Extern for the harness function (matches harness signature exactly)
void jpeg_idct_islow (j_decompress_ptr cinfo, jpeg_component_info * compptr,
                      JCOEFPTR coef_block, JSAMPARRAY output_buf, JDIMENSION output_col);

int main() {
    j_decompress_ptr cinfo = (j_decompress_ptr)0; // unused in harness body

    jpeg_component_info comp;
    comp.dct_table = malloc(128);

    // Coefficient block (64 coefficients)
    JCOEFPTR coef_block = (JCOEFPTR)malloc(64 * sizeof(*coef_block));

    // Output buffer: 8 rows of 8 samples each
    JSAMPARRAY output_buf = (JSAMPARRAY)malloc(8 * sizeof(*output_buf));
    for (int i = 0; i < 8; ++i) {
        output_buf[i] = (JSAMPROW)malloc(8 * sizeof(*output_buf[i]));
    }

    JDIMENSION output_col = 0;

    jpeg_idct_islow(cinfo, &comp, coef_block, output_buf, output_col);
    return 0;
}
