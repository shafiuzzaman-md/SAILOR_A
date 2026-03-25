#include <string.h>
#include "harness_types.h"
#include <stdlib.h>
// klee removed for replay

int main() {
    // Allocate output buffer: DCTSIZE is 8 in harness; indices used: 0,1,2,3,4,6
    DCTELEM *data = (DCTELEM *)calloc(8, sizeof(DCTELEM));

    // Allocate one row in sample_data; function accesses sample_data[0] only
    const size_t rows = 1;
    JSAMPARRAY sample_data = (JSAMPARRAY)calloc(rows, sizeof(JSAMPROW));

    // Concrete row length; access uses elemptr[0..8], so start_col >= 1 will make elemptr[8] OOB
    const size_t row_len = 9;  // concrete size per instructions
    JSAMPROW row0 = (JSAMPROW)malloc(row_len);
    { static const unsigned char row0_bytes_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(row0, row0_bytes_data, (row_len < sizeof(row0_bytes_data)) ? row_len : sizeof(row0_bytes_data)); };

    sample_data[0] = row0;

    // Make start_col symbolic and constrain to force the OOB path on elemptr[8]
    JDIMENSION start_col = 0; // keep early elemptr[8] access in-bounds to bypass non-target crash

    // Call entry/vulnerable function directly
    jpeg_fdct_9x9(data, sample_data, start_col);

    return 0;
}
