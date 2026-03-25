#include <stddef.h>
#include <string.h>
#include "harness_types.h"
// klee removed for replay
#include <stdlib.h>
#include <stdint.h>

int LLVMFuzzerTestOneInput(const uint8_t *fuzz_data, size_t fuzz_size) {
    if (fuzz_size < 6) return 0;
    // Allocate data buffer (not used by our neutralized harness, but must be non-NULL)
    DCTELEM *data = (DCTELEM *)malloc(sizeof(DCTELEM) * 64);

    // Prepare JSAMPARRAY with a single row
    JSAMPARRAY sample_data = (JSAMPARRAY)malloc(sizeof(JSAMPROW) * 1);

    // Allocate a too-small row: less than 7 bytes so elemptr[6] is OOB
    const size_t row_len = 6; // OOB at index 6
    JSAMPROW row = (JSAMPROW)malloc(row_len);

    // Make row content symbolic (size remains concrete)
    { memcpy(row, fuzz_data + 0, 6); };

    sample_data[0] = row;

    // start_col = 0 so accesses are row[4] and row[6]
    JDIMENSION start_col = 0;

    jpeg_fdct_11x11(data, sample_data, start_col);
    return 0;
}
