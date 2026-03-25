#include <stddef.h>
#include <string.h>
#include "harness_types.h"
// klee removed for replay
#include <stdlib.h>
#include <stdint.h>

int LLVMFuzzerTestOneInput(const uint8_t *fuzz_data, size_t fuzz_size) {
    if (fuzz_size < 1) return 0;
    // Allocate output coefficient buffer (concrete size)
    DCTELEM *data = (DCTELEM *)calloc(64, sizeof(DCTELEM));

    // Allocate one row pointer (JSAMPARRAY with 1 JSAMPROW)
    JSAMPARRAY sample_data = (JSAMPARRAY)calloc(1, sizeof(JSAMPROW));

    // Allocate a too-short row to trigger out-of-bounds reads (length 1 byte)
    size_t row_len = 16; // concrete, non-zero
    JSAMPROW row = (JSAMPROW)malloc(row_len);

    // Make the row contents symbolic (size must be concrete and > 0)
    { memcpy(row, fuzz_data + 0, 1); };

    // Hook up the row into the array
    sample_data[0] = row;

    // Start at column 0 so elemptr[11] is out-of-bounds
    JDIMENSION start_col = 0;

    // Direct call to the entry/vulnerable function
    jpeg_fdct_12x12(data, sample_data, start_col);

    return 0;
}
