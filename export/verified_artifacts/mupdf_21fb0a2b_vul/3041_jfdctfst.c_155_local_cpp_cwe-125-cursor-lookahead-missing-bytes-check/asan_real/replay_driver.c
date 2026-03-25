#include "harness_types.h"
// klee removed for replay
#include <stdlib.h>
#include <string.h>

extern void jpeg_fdct_ifast (DCTELEM * data, JSAMPARRAY sample_data, JDIMENSION start_col);

int main() {
    /* Workspace for outputs: need at least 8 DCTELEM entries for indices 0,2,4,6 */
    DCTELEM *data = (DCTELEM *)calloc(8, sizeof(DCTELEM));

    /* Prepare JSAMPARRAY for input rows */
    JSAMPARRAY sample_data = (JSAMPARRAY)calloc(8, sizeof(JSAMPROW));

    /* Provide a sufficiently large row to avoid early OOB on elemptr[0..7] after start_col offset */
    const size_t row_len = 16; /* concrete; allows start_col up to 8 */
    JSAMPROW row0 = (JSAMPROW)malloc(row_len);
    { static const unsigned char row0_bytes_data[] = {0x00, 0x00, 0x00, 0x00}; memcpy(row0, row0_bytes_data, (row_len < sizeof(row0_bytes_data)) ? row_len : sizeof(row0_bytes_data)); };

    sample_data[0] = row0;

    JDIMENSION start_col;
    { static const unsigned char start_col_data[] = {0x00, 0x00, 0x00, 0x00}; memcpy(&start_col, start_col_data, (sizeof(start_col) < sizeof(start_col_data)) ? sizeof(start_col) : sizeof(start_col_data)); };
    /* Constrain so that (start_col + 7) < row_len to avoid non-target OOB before the sink */
    /* klee_assume removed */

    jpeg_fdct_ifast(data, sample_data, start_col);
    return 0;
}
