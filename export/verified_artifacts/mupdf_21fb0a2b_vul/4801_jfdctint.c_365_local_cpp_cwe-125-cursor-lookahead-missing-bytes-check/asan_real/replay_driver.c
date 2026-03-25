#include <string.h>
// NO_HARNESS_TYPES
#include <stdlib.h>
#include <stdint.h>
// klee removed for replay

/* Minimal local typedefs matching libjpeg semantics */
typedef unsigned char JSAMPLE;
typedef JSAMPLE* JSAMPROW;
typedef JSAMPROW* JSAMPARRAY;
typedef unsigned int JDIMENSION;
typedef int DCTELEM;

/* Prototype for the target function (defined in harness/parser.c) */
extern void jpeg_fdct_7x7(DCTELEM *data, JSAMPARRAY sample_data, JDIMENSION start_col);

int main(void) {
    /* Allocate DCT output buffer (concrete size) */
    DCTELEM *data = (DCTELEM *)calloc(64, sizeof(DCTELEM));

    /* Set up sample_data as 7 rows with very short row length to trigger elemptr[6] OOB */
    const int rows = 7;
    const int row_len = 2; /* small so that accessing index 6 is out-of-bounds */

    JSAMPARRAY sample_data = (JSAMPARRAY)calloc(rows, sizeof(JSAMPROW));
    for (int r = 0; r < rows; r++) {
        sample_data[r] = (JSAMPROW)malloc(row_len * sizeof(JSAMPLE));
        /* Make row contents symbolic */
        { static const unsigned char row_bytes_data[] = {0x00, 0x00}; memcpy(sample_data[r], row_bytes_data, (row_len * sizeof(JSAMPLE) < sizeof(row_bytes_data)) ? row_len * sizeof(JSAMPLE) : sizeof(row_bytes_data)); };
    }

    /* Make start_col symbolic but within a tiny range; any value here will still cause OOB at [6] */
    JDIMENSION start_col;
    { static const unsigned char start_col_data[] = {0x00, 0x00, 0x00, 0x00}; memcpy(&start_col, start_col_data, (sizeof(start_col) < sizeof(start_col_data)) ? sizeof(start_col) : sizeof(start_col_data)); };
    /* klee_assume removed */ /* ensures start_col + 6 >= 7 > row_len (2) */

    /* Direct call to target function */
    jpeg_fdct_7x7(data, sample_data, start_col);

    return 0;
}
