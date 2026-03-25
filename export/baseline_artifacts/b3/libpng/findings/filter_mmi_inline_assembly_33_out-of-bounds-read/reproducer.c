#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Minimal redeclarations from libpng */
typedef unsigned char png_byte;
typedef struct {
    size_t rowbytes;
} png_row_info;

/* Reimplementation of the vulnerable function logic in portable C.
 * It mimics the MIPS inline assembly's behavior of processing 8 bytes per
 * iteration unconditionally, without handling any remainder. This causes
 * an out-of-bounds read from prev_row when rowbytes is not a multiple of 8. */
void png_read_filter_row_up_mmi(png_row_info *row_info, png_byte *row,
                                const png_byte *prev_row)
{
    int istop = (int)row_info->rowbytes;
    while (istop > 0) {
        /* Unconditionally read 8 bytes from both row and prev_row. */
        uint64_t rp, pp;
        /* Using memcpy ensures an 8-byte read regardless of remaining bytes,
         * which ASan will check and report as OOB when insufficient. */
        memcpy(&rp, row, 8);
        memcpy(&pp, prev_row, 8);  /* OOB read occurs here when <8 bytes remain */

        rp += pp; /* Not byte-wise add like paddb, but irrelevant for triggering OOB */
        memcpy(row, &rp, 8);       /* Also writes 8 bytes */

        row      += 8;
        prev_row += 8;
        istop    -= 8;
    }
}

int main(void)
{
    /* Choose a rowbytes that is NOT a multiple of 8 to force a remainder.
     * Two 8-byte iterations will be done for 13 bytes: second iteration
     * reads bytes [8..15] from prev_row, which is OOB by 3 bytes. */
    const size_t rowbytes = 13; /* 13 % 8 = 5 remainder */

    /* Allocate prev_row with exactly rowbytes, so the second 8-byte load
     * goes past the allocation and ASan reports an out-of-bounds read. */
    png_byte *prev_row = (png_byte *)malloc(rowbytes);
    if (!prev_row) {
        perror("malloc prev_row");
        return 1;
    }

    /* Allocate row with extra padding so the 8-byte store in the final
     * iteration does not itself trigger an OOB write first. This isolates
     * the OOB read from prev_row as the primary report. */
    png_byte *row = (png_byte *)malloc(rowbytes + 8);
    if (!row) {
        perror("malloc row");
        free(prev_row);
        return 1;
    }

    /* Initialize buffers with known data. */
    for (size_t i = 0; i < rowbytes; ++i) {
        prev_row[i] = (png_byte)(i & 0xFF);
        row[i] = (png_byte)((0x80 + i) & 0xFF);
    }
    /* Initialize the padding region of row to avoid uninitialized warnings. */
    memset(row + rowbytes, 0xAA, 8);

    png_row_info info;
    info.rowbytes = rowbytes;

    /* Call the vulnerable function. With rowbytes=13, it will perform:
     * - Iteration 1: read/write 8 bytes OK
     * - Iteration 2: attempt to read/write another 8 bytes
     *   -> prev_row read goes beyond its 13-byte allocation (OOB read) */
    png_read_filter_row_up_mmi(&info, row, prev_row);

    /* Clean up (the program may already have crashed/aborted due to ASan). */
    free(row);
    free(prev_row);

    return 0;
}
