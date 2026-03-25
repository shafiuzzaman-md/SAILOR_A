#include <string.h>
#include "harness_types.h"
#include <stdlib.h>
// klee removed for replay

#ifndef DCTSIZE
#define DCTSIZE 8
#endif

// Forward declaration for the target function
void jpeg_fdct_16x8 (DCTELEM * data, JSAMPARRAY sample_data, JDIMENSION start_col);

int main() {
    // Allocate output coefficient buffer (8x8 block)
    const size_t data_elems = DCTSIZE * DCTSIZE; // 64
    DCTELEM *data = (DCTELEM *)calloc(data_elems, sizeof(DCTELEM));
    if (!data) return 1;

    // Allocate JSAMPARRAY with 8 rows; each row has 16 samples (accessed 0..15)
    JSAMPARRAY sample_data = (JSAMPARRAY)malloc(DCTSIZE * sizeof(JSAMPROW));
    if (!sample_data) return 1;

    for (int r = 0; r < DCTSIZE; ++r) {
        JSAMPROW row = (JSAMPROW)malloc(16 * sizeof(JSAMPLE));
        if (!row) return 1;
        // Make row bytes symbolic so KLEE can explore paths
        char name[16] = {0};
        name[0] = 'r'; name[1] = (char)('0' + r);
        memset(row, 0, 16 * sizeof(JSAMPLE)); /* replay: no ktest data for "name" */;
        sample_data[r] = row;
    }

    // start_col controls lookahead; make it symbolic
    JDIMENSION start_col;
    { static const unsigned char start_col_data[] = {0x20, 0x00, 0x00, 0x00}; memcpy(&start_col, start_col_data, (sizeof(start_col) < sizeof(start_col_data)) ? sizeof(start_col) : sizeof(start_col_data)); };
    // Keep within a small range to allow potential OOB on 16-byte rows
    /* klee_assume removed */

    // Call the vulnerable function directly
    jpeg_fdct_16x8(data, sample_data, start_col);

    return 0;
}
