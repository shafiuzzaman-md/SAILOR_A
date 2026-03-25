#include <string.h>
#include "harness_types.h"
// klee removed for replay
#include <stdlib.h>
#include <stdint.h>

int main() {
    // Allocate some DCT output buffer (size arbitrary in this slice)
    DCTELEM *data = (DCTELEM *)calloc(64, sizeof(DCTELEM));

    // Prepare JSAMPARRAY with a single row pointer
    JSAMPARRAY sample_data = (JSAMPARRAY)malloc(sizeof(JSAMPROW));

    // Symbolic row length; force <= 6 so elemptr[6] is out-of-bounds
    unsigned row_len;
    { static const unsigned char row_len_data[] = {0x01, 0x00, 0x00, 0x00}; memcpy(&row_len, row_len_data, (sizeof(row_len) < sizeof(row_len_data)) ? sizeof(row_len) : sizeof(row_len_data)); };
    /* klee_assume removed */

    // Allocate at least 1 byte to avoid malloc(0)
    unsigned alloc_len = (row_len == 0) ? 1u : row_len;
    JSAMPROW row = (JSAMPROW)malloc(alloc_len);
    { static const unsigned char row_bytes_data[] = {0x00}; memcpy(row, row_bytes_data, (alloc_len < sizeof(row_bytes_data)) ? alloc_len : sizeof(row_bytes_data)); };
    sample_data[0] = row;

    // Constrain start_col to 0 so vulnerable access uses row indices directly
    JDIMENSION start_col = 0;

    // Call entry/vulnerable function directly
    jpeg_fdct_islow(data, sample_data, start_col);
    return 0;
}
