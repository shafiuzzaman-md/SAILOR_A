#include "harness_types.h"
// klee removed for replay
#include <stdlib.h>
#include <string.h>

int main() {
    // Allocate data buffer (unused in slice but required by signature)
    DCTELEM *data = (DCTELEM *)malloc(64 * sizeof(DCTELEM));
    if (!data) return 0;
    memset(data, 0, 64 * sizeof(DCTELEM));

    // Allocate one JSAMPROW (row) with too few bytes to trigger OOB on elemptr[7]
    // Accesses: elemptr[5] and elemptr[7] => need < 8 bytes to go OOB
    size_t row_len = 4; // concrete small size ensures OOB on index 5/7
    JSAMPROW row = (JSAMPROW)malloc(row_len);
    if (!row) return 0;
    { static const unsigned char row_bytes_data[] = {0x00, 0x00, 0x00, 0x00}; memcpy(row, row_bytes_data, (row_len < sizeof(row_bytes_data)) ? row_len : sizeof(row_bytes_data)); };

    // JSAMPARRAY with a single row
    JSAMPARRAY sample_data = (JSAMPARRAY)malloc(sizeof(JSAMPROW) * 1);
    if (!sample_data) return 0;
    sample_data[0] = row;

    // start_col at 0 so elemptr points to start of row
    JDIMENSION start_col = 0;

    // Direct call to entry/vulnerable function
    jpeg_fdct_13x13(data, sample_data, start_col);

    return 0;
}
