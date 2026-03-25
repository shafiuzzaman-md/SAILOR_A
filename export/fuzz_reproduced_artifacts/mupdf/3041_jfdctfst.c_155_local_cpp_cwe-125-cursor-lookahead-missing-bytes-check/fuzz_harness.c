#include <stdint.h>
#include <stddef.h>
#include "harness_types.h"
// klee removed for replay
#include <stdlib.h>
#include <string.h>

extern void jpeg_fdct_ifast (DCTELEM * data, JSAMPARRAY sample_data, JDIMENSION start_col);

int LLVMFuzzerTestOneInput(const uint8_t *fuzz_data, size_t fuzz_size) {
    if (fuzz_size < 4) return 0;
    /* Workspace for outputs: need at least 8 DCTELEM entries for indices 0,2,4,6 */
    DCTELEM *data = (DCTELEM *)calloc(8, sizeof(DCTELEM));

    /* Prepare JSAMPARRAY for input rows */
    JSAMPARRAY sample_data = (JSAMPARRAY)calloc(8, sizeof(JSAMPROW));

    /* Provide a sufficiently large row to avoid early OOB on elemptr[0..7] after start_col offset */
    const size_t row_len = 16; /* concrete; allows start_col up to 8 */
    JSAMPROW row0 = (JSAMPROW)malloc(row_len);
    { memcpy(row0, fuzz_data + 0, 4); };

    sample_data[0] = row0;

    JDIMENSION start_col;
    { static const unsigned char start_col_data[] = {0x00, 0x00, 0x00, 0x00}; memcpy(&start_col, start_col_data, (sizeof(start_col) < sizeof(start_col_data)) ? sizeof(start_col) : sizeof(start_col_data)); };
    /* Constrain so that (start_col + 7) < row_len to avoid non-target OOB before the sink */
    /* klee_assume removed */

    jpeg_fdct_ifast(data, sample_data, start_col);
    return 0;
}
