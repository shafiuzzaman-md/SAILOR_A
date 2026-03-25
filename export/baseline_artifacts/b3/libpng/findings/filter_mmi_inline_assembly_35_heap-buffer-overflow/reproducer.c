#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Minimal stand-ins for libpng types */
typedef unsigned char png_byte;

typedef struct png_row_info {
    size_t rowbytes;
} png_row_info;

/*
 * Buggy implementation modeled after the MIPS inline assembly in
 * png_read_filter_row_up_mmi: it processes 8 bytes per iteration and
 * always performs an 8-byte store without handling a tail shorter than 8.
 */
__attribute__((noinline))
void png_read_filter_row_up_mmi(png_row_info *row_info, png_byte *row,
                                const png_byte *prev_row)
{
    int istop = (int)row_info->rowbytes;

    for (;;) {
        /* Emulate parallel byte add for 8 bytes. The last (tail) iteration
         * will read and write 8 bytes unconditionally, overflowing if
         * rowbytes is not a multiple of 8.
         */
        unsigned char out[8];
        for (int i = 0; i < 8; ++i) {
            /* These indexed accesses intentionally read 8 bytes regardless of
             * remaining length, mirroring the buggy ldc1/sdc1 behavior. */
            out[i] = (unsigned char)(row[i] + prev_row[i]);
        }

        /* Unconditional 8-byte store (can overflow when tail < 8) */
        memcpy(row, out, 8);

        row      += 8;
        prev_row += 8;
        istop    -= 8;
        if (istop <= 0)
            break; /* Branch after the 8-byte store, like the assembly */
    }
}

int main(void)
{
    /* Choose a row size not divisible by 8 to trigger the overflow. */
    const size_t rowbytes = 9; /* Tail = 1 byte, store still writes 8 bytes */

    png_row_info info;
    info.rowbytes = rowbytes;

    png_byte *row = (png_byte *)malloc(rowbytes);
    png_byte *prev = (png_byte *)malloc(rowbytes);

    if (!row || !prev) {
        fprintf(stderr, "Allocation failed\n");
        return 1;
    }

    /* Initialize buffers with deterministic data */
    for (size_t i = 0; i < rowbytes; ++i) {
        row[i] = (png_byte)(i & 0xFF);
        prev[i] = (png_byte)((0xF0 + i) & 0xFF);
    }

    /* This call will perform an 8-byte write past the end of 'row' on the
     * last iteration, causing a heap-buffer-overflow under ASan. */
    png_read_filter_row_up_mmi(&info, row, prev);

    /* Prevent the compiler from optimizing away the call/uses */
    volatile unsigned char sink = row[0];
    (void)sink;

    free(row);
    free(prev);
    return 0;
}
