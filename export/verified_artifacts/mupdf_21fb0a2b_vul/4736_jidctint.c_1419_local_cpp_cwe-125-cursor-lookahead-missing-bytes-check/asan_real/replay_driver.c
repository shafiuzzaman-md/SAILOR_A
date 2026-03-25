// NO_HARNESS_TYPES
// klee removed for replay
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

// Local replicas matching harness/jidctint.c
#ifndef JDIMENSION
#define JDIMENSION unsigned int
#endif

typedef int INT32;

typedef struct jpeg_decompress_struct { int dummy; } * j_decompress_ptr;

typedef struct jpeg_component_info {
    void *dct_table;
} jpeg_component_info;

typedef short JCOEF;
typedef JCOEF * JCOEFPTR;

typedef unsigned char JSAMPLE;
typedef JSAMPLE * JSAMPROW;
typedef JSAMPROW * JSAMPARRAY;

// extern of target function with 5 params (matches harness)
extern void jpeg_idct_10x10(j_decompress_ptr cinfo, jpeg_component_info * compptr,
                            JCOEFPTR coef_block, JSAMPARRAY output_buf, JDIMENSION output_col);

int main() {
    // Allocate context structs
    j_decompress_ptr cinfo = (j_decompress_ptr)calloc(1, sizeof(*cinfo));
    jpeg_component_info *compptr = (jpeg_component_info *)calloc(1, sizeof(*compptr));
    if (!cinfo || !compptr) return 0;

    // Workspace (wsptr) base: allocate only 4 INT32 elements so wsptr[4] is OOB
    size_t wlen = 4; // fewer than 5 to exercise lookahead read
    INT32 *workspace = (INT32 *)malloc(wlen * sizeof(INT32));
    if (!workspace) return 0;
    { static const unsigned char workspace_bytes_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(workspace, workspace_bytes_data, (wlen * sizeof(INT32) < sizeof(workspace_bytes_data)) ? wlen * sizeof(INT32) : sizeof(workspace_bytes_data)); };

    // Satisfy entry precondition: quantptr = (ISLOW_MULT_TYPE *) compptr->dct_table
    compptr->dct_table = (void *)workspace;

    // Coefficient block (unused here but valid pointer)
    JCOEFPTR coef_block = (JCOEFPTR)calloc(64, sizeof(*coef_block));
    if (!coef_block) return 0;
    { static const unsigned char coef_block_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(coef_block, coef_block_data, (64 * sizeof(*coef_block) < sizeof(coef_block_data)) ? 64 * sizeof(*coef_block) : sizeof(coef_block_data)); };

    // Output buffer: 10 rows as iterated by the loop
    int rows = 10;
    JSAMPARRAY output_buf = (JSAMPARRAY)calloc(rows, sizeof(JSAMPROW));
    if (!output_buf) return 0;
    for (int i = 0; i < rows; i++) {
        output_buf[i] = (JSAMPROW)malloc(1); // minimal row
        if (!output_buf[i]) return 0;
        output_buf[i][0] = 0;
    }

    JDIMENSION output_col = 0;

    // Direct call to entry/vulnerable function
    jpeg_idct_10x10(cinfo, compptr, coef_block, output_buf, output_col);

    return 0;
}
