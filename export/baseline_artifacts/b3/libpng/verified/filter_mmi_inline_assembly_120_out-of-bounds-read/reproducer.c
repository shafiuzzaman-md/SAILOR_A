#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Minimal libpng-like typedefs */
typedef unsigned char png_byte;
typedef struct {
    uint32_t rowbytes;
} png_row_info;

/* Buggy implementation mirroring the MIPS inline-asm logic:
 * Processes 4-byte chunks and, in each iteration, reads 4 bytes at row+4
 * and writes the result back at row+4. When only 4 bytes remain (istop==4),
 * the read from row+4 goes past the end of the buffer (OOB read).
 */
void png_read_filter_row_sub4_mmi(png_row_info *row_info, png_byte *row, const png_byte *prev)
{
    int istop = (int)row_info->rowbytes;

    while (istop > 0) {
        uint32_t pp = 0, rp = 0;
        /* Read current 4-byte block at row */
        memcpy(&pp, row + 0, 4);
        /* OOB read when istop == 4: attempts to read 4 bytes from row+4 */
        memcpy(&rp, row + 4, 4);
        /* Approximate paddb with simple addition (details don't matter for OOB) */
        rp = rp + pp;
        /* Write result back to row+4 (also OOB when istop == 4) */
        memcpy(row + 4, &rp, 4);

        row += 4;
        istop -= 4;
    }

    (void)prev; /* Unused in this context */
}

int main(void)
{
    /* Set up a row buffer with exactly 4 bytes so the loop runs once and
     * the function reads from row+4 (past-the-end) in that iteration.
     */
    png_row_info info;
    info.rowbytes = 4;  /* exactly one 4-byte chunk */

    png_byte *row = (png_byte *)malloc(info.rowbytes);
    if (!row) {
        perror("malloc");
        return 1;
    }

    /* Initialize row with some data */
    for (uint32_t i = 0; i < info.rowbytes; ++i) row[i] = (png_byte)(i + 1);

    /* Trigger the bug: this will perform an out-of-bounds read from row+4 */
    png_read_filter_row_sub4_mmi(&info, row, NULL);

    /* Touch the buffer so the compiler doesn't optimize everything away */
    volatile unsigned char sink = row[0];
    printf("sink=%u\n", (unsigned)sink);

    free(row);
    return 0;
}