#include "harness_types.h"
// klee removed for replay
#include <stdlib.h>
#include <string.h>

extern void jpeg_fdct_10x5 (DCTELEM * data, JSAMPARRAY sample_data, JDIMENSION start_col);

int main() {
    // Allocate output buffer (only index 0 is written in the sliced function)
    DCTELEM data_buf[1] = {0};

    // Provide enough bytes so elemptr[0..9] are in-bounds to avoid early OOB
    unsigned char row_storage[16];
    { static const unsigned char row0_data[] = {0x00, 0x00, 0x00, 0x00}; memcpy(row_storage, row0_data, (sizeof(row_storage) < sizeof(row0_data)) ? sizeof(row_storage) : sizeof(row0_data)); };

    JSAMPROW row0 = (JSAMPROW)row_storage;
    JSAMPROW rows[1];
    rows[0] = row0;

    JSAMPARRAY sample_data = (JSAMPARRAY)rows;

    JDIMENSION start_col = 0;

    jpeg_fdct_10x5(data_buf, sample_data, start_col);
    return 0;
}
