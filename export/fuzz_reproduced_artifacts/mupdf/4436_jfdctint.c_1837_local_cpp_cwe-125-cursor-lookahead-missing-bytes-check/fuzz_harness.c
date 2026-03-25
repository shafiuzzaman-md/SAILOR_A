#include <stdint.h>
#include <stddef.h>
#include "harness_types.h"
// klee removed for replay
#include <stdlib.h>
#include <string.h>

int LLVMFuzzerTestOneInput(const uint8_t *fuzz_data, size_t fuzz_size) {
    if (fuzz_size < 136) return 0;
    // Allocate data buffer (not used by our slice, but keep real object)
    DCTELEM *data = (DCTELEM *)malloc(sizeof(DCTELEM) * 64);
    if (!data) return 0;
    { memcpy(data, fuzz_data + 0, 128); };

    // Allocate one-row JSAMPARRAY
    JSAMPARRAY sample_data = (JSAMPARRAY)malloc(sizeof(JSAMPROW) * 1);
    if (!sample_data) return 0;

    // Allocate a small row so that start_col + 7 can exceed its bounds
    const size_t ROW_SIZE = 8; // exactly 8 bytes; start_col >= 1 will cause OOB at elemptr[7]
    JSAMPROW row0 = (JSAMPROW)malloc(ROW_SIZE);
    if (!row0) return 0;
    { memcpy(row0, fuzz_data + 128, 8); };

    sample_data[0] = row0;

    // start_col is symbolic; KLEE can pick values that cause OOB
    JDIMENSION start_col;
    { static const unsigned char start_col_data[] = {0x00, 0x00, 0x00, 0x00}; memcpy(&start_col, start_col_data, (sizeof(start_col) < sizeof(start_col_data)) ? sizeof(start_col) : sizeof(start_col_data)); };
    // Keep range reasonable to avoid huge search; still allows OOB (e.g., 1)
    /* klee_assume removed */

    // Call entry function directly (no guards)
    jpeg_fdct_14x14(data, sample_data, start_col);

    return 0;
}
