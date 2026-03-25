#include <string.h>
#include "harness_types.h"
#include <stdlib.h>
// klee removed for replay

// Prototype from harness
void jpeg_fdct_8x16 (DCTELEM * data, JSAMPARRAY sample_data, JDIMENSION start_col);

int main() {
    // Allocate output DCT buffer (concrete size)
    DCTELEM *data = (DCTELEM *)calloc(DCTSIZE2, sizeof(DCTELEM));

    // Allocate JSAMPARRAY with a single row
    JSAMPARRAY sample_data = (JSAMPARRAY)malloc(sizeof(JSAMPROW));

    // Allocate a short row buffer (< 8 bytes) to induce OOB when accessing elemptr[7]
    size_t row_len = 8; // concrete, intentionally smaller than 8
    JSAMPROW row0 = (JSAMPROW)malloc(row_len);
    { static const unsigned char row0_bytes_data[] = {0x00, 0x00, 0x00, 0x00}; memcpy(row0, row0_bytes_data, (row_len < sizeof(row0_bytes_data)) ? row_len : sizeof(row0_bytes_data)); };

    sample_data[0] = row0;

    // Start at column 0 to ensure elemptr[7] is out-of-bounds for row_len=4
    JDIMENSION start_col = 0;

    // Call entry/vulnerable function
    jpeg_fdct_8x16(data, sample_data, start_col);
    return 0;
}
