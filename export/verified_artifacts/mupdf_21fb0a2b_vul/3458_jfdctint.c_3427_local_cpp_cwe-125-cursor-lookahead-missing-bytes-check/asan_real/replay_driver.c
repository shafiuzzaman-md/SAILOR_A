#include "harness_types.h"
// klee removed for replay
#include <stdlib.h>
#include <string.h>

int main() {
    // Allocate data buffer (unused by our slice but required by signature)
    DCTELEM *data = (DCTELEM*)calloc(64, sizeof(DCTELEM));

    // Allocate one row with only 4 bytes so elemptr[4] (and [7]) is OOB
    size_t row_len = 4; // concrete small size to force OOB on lookahead
    JSAMPROW row0 = (JSAMPROW)malloc(row_len);
    { static const unsigned char row0_bytes_data[] = {0x00, 0x00, 0x00, 0x00}; memcpy(row0, row0_bytes_data, (row_len < sizeof(row0_bytes_data)) ? row_len : sizeof(row0_bytes_data)); };

    // JSAMPARRAY with a single row
    JSAMPARRAY sample_data = (JSAMPARRAY)malloc(sizeof(JSAMPROW) * 1);
    sample_data[0] = row0;

    // start_col = 0 ensures deref of elemptr[4] is out of bounds
    JDIMENSION start_col = 0;

    // Direct call to entry/vulnerable function
    jpeg_fdct_8x16(data, sample_data, start_col);

    return 0;
}
