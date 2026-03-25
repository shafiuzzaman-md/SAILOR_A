#include <stddef.h>
#include "harness_types.h"
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
// klee removed for replay

#ifndef DCTSIZE
#define DCTSIZE 8
#endif

// Prototype from harness
extern void jpeg_fdct_ifast (DCTELEM * data, JSAMPARRAY sample_data, JDIMENSION start_col);

int LLVMFuzzerTestOneInput(const uint8_t *fuzz_data, size_t fuzz_size) {
    if (fuzz_size < 128) return 0;
    // Allocate output buffer: 8x8 DCT coefficients
    DCTELEM *data = (DCTELEM *)malloc(sizeof(DCTELEM) * DCTSIZE * DCTSIZE);
    if (!data) return 0;
    { memcpy(data, fuzz_data + 0, 128); };

    // Allocate input sample rows: 8 rows, each with a concrete length
    const size_t row_len = 16;  // concrete size
    JSAMPARRAY sample_data = (JSAMPARRAY)malloc(sizeof(JSAMPROW) * DCTSIZE);
    if (!sample_data) return 0;
    for (int i = 0; i < DCTSIZE; ++i) {
        sample_data[i] = (JSAMPROW)malloc(row_len * sizeof(JSAMPLE));
        if (!sample_data[i]) return 0;
        { static const unsigned char row_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(sample_data[i], row_data, (row_len * sizeof(JSAMPLE) < sizeof(row_data)) ? row_len * sizeof(JSAMPLE) : sizeof(row_data)); };
    }

    // start_col is symbolic; no in-bounds constraint so KLEE can explore OOB
    JDIMENSION start_col;
    { static const unsigned char start_col_data[] = {0x10, 0x00, 0x00, 0x00}; memcpy(&start_col, start_col_data, (sizeof(start_col) < sizeof(start_col_data)) ? sizeof(start_col) : sizeof(start_col_data)); };

    // Call entry/vulnerable function directly
    jpeg_fdct_ifast(data, sample_data, start_col);

    return 0;
}
