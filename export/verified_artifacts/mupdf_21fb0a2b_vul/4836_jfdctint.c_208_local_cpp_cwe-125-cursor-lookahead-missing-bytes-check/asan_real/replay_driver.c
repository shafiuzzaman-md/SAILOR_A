#include <string.h>
// NO_HARNESS_TYPES
// klee removed for replay

/* Local minimal type defs matching harness preamble */
typedef short DCTELEM;
typedef unsigned int JDIMENSION;
typedef unsigned char JSAMPLE;
typedef JSAMPLE *JSAMPROW;
typedef JSAMPROW *JSAMPARRAY;

/* Declare target function */
void jpeg_fdct_islow (DCTELEM * data, JSAMPARRAY sample_data, JDIMENSION start_col);

/* Minimal malloc prototype to avoid including stdlib.h */
extern void *malloc(unsigned long);

int main() {
    /* Allocate a too-small buffer so dataptr[6] goes OOB */
    unsigned long n_elems = 4; /* < 7 to force OOB at index 6 */
    DCTELEM *data = (DCTELEM*)malloc(n_elems * sizeof(DCTELEM));
    if (!data) return 0;
    { static const unsigned char data_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(data, data_data, (n_elems * sizeof(DCTELEM) < sizeof(data_data)) ? n_elems * sizeof(DCTELEM) : sizeof(data_data)); };

    /* sample_data and start_col are unused by our sliced harness */
    JSAMPARRAY sample_data = 0;
    JDIMENSION start_col = 0;

    jpeg_fdct_islow(data, sample_data, start_col);
    return 0;
}
