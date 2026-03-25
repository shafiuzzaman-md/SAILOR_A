#include <string.h>
// NO_HARNESS_TYPES
// klee removed for replay
#include <stdlib.h>
#include <stdint.h>

/* Minimal matching typedefs consistent with the harness */
typedef unsigned int JDIMENSION;
typedef unsigned char JSAMPLE;
typedef JSAMPLE* JSAMPROW;
typedef JSAMPROW* JSAMPARRAY;
typedef short JCOEF;
typedef JCOEF* JCOEFPTR;

typedef struct jpeg_decompress_struct { int dummy; } jpeg_decompress_struct;
typedef jpeg_decompress_struct* j_decompress_ptr;

typedef struct jpeg_component_info { void *dct_table; } jpeg_component_info;

/* Explicit prototype matching the harness definition (5 params) */
extern void jpeg_idct_3x3 (j_decompress_ptr cinfo, jpeg_component_info * compptr,
                           JCOEFPTR coef_block, JSAMPARRAY output_buf,
                           JDIMENSION output_col);

int main() {
    // Allocate a JSAMPARRAY with a single row
    JSAMPARRAY output_buf = (JSAMPARRAY)malloc(sizeof(JSAMPROW) * 1);
    if (!output_buf) return 0;

    // Allocate a too-small row (only 2 bytes), so outptr[2] is OOB
    JSAMPROW row0 = (JSAMPROW)malloc(2);
    if (!row0) return 0;
    { static const unsigned char row0_bytes_data[] = {0x00, 0x00}; memcpy(row0, row0_bytes_data, (2 < sizeof(row0_bytes_data)) ? 2 : sizeof(row0_bytes_data)); };

    output_buf[0] = row0;

    // Other parameters (unused by harnessed function body)
    j_decompress_ptr cinfo = (j_decompress_ptr)0;
    jpeg_component_info *compptr = (jpeg_component_info *)0;
    JCOEFPTR coef_block = (JCOEFPTR)0;
    JDIMENSION output_col = 0; // start at beginning of row

    // Call entry/vulnerable function directly
    jpeg_idct_3x3(cinfo, compptr, coef_block, output_buf, output_col);

    return 0;
}
