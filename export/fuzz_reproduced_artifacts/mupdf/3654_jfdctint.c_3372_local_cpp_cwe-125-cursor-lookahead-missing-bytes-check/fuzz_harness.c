#include <stdint.h>
#include <stddef.h>
#include "harness_types.h"
// klee removed for replay
#include <stdlib.h>
#include <string.h>

int LLVMFuzzerTestOneInput(const uint8_t *fuzz_data, size_t fuzz_size) {
    if (fuzz_size < 1) return 0;
    // Allocate output coefficient block
    DCTELEM data[DCTSIZE2];

    // Prepare sample_data with exactly one JSAMPROW of length 1 to force OOB on elemptr[1]
    JSAMPARRAY sample_data = (JSAMPARRAY)malloc(sizeof(JSAMPROW));
    if (!sample_data) return 0;

    JSAMPROW row0 = (JSAMPROW)malloc(1); // only 1 byte available
    if (!row0) return 0;
    { memcpy(row0, fuzz_data + 0, 1); }; // content symbolic

    sample_data[0] = row0;

    // Start at column 0 so elemptr = row0 + 0; elemptr[0] valid, elemptr[1] OOB
    JDIMENSION start_col = 0;

    // Call entry/vulnerable function directly
    jpeg_fdct_2x1(data, sample_data, start_col);

    return 0;
}
