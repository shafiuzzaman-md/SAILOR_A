#include <string.h>
#include "harness_types.h"
#include <stdlib.h>
// klee removed for replay

// Prototype for the target function implemented in the harness
extern void jpeg_fdct_9x9(DCTELEM * data, JSAMPARRAY sample_data, JDIMENSION start_col);

int main(void) {
    const int dim = 9;
    DCTELEM *data = (DCTELEM *)calloc(dim * dim, sizeof(DCTELEM));

    // Prepare sample_data as an array of 9 row pointers
    // Intentionally allocate rows of length 8 to trigger OOB on elemptr[8]
    const int rows = 9;
    const int row_len = 8; // insufficient: accessing [8] goes OOB

    JSAMPARRAY sample_data = (JSAMPARRAY)calloc(rows, sizeof(JSAMPROW));
    for (int i = 0; i < rows; i++) {
        sample_data[i] = (JSAMPROW)malloc(row_len * sizeof(JSAMPLE));
        { static const unsigned char row_bytes_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(sample_data[i], row_bytes_data, (row_len * sizeof(JSAMPLE) < sizeof(row_bytes_data)) ? row_len * sizeof(JSAMPLE) : sizeof(row_bytes_data)); };
    }

    JDIMENSION start_col = 0; // ensures elemptr[8] targets the 9th byte

    // Call the vulnerable function directly
    jpeg_fdct_9x9(data, sample_data, start_col);

    return 0;
}
