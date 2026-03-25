// Standalone reproducer for heap-buffer-overflow in png_read_filter_row_sub4_mmi
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

// Minimal type definitions to match the vulnerable function's signature
typedef unsigned char png_byte;
typedef struct png_row_info_s {
    size_t width;
    size_t rowbytes;
} png_row_info;

// Reimplementation of the vulnerable function logic in portable C.
// It mirrors the MIPS inline assembly loop that:
// - Processes 4 bytes per iteration
// - Loads from row and row+4
// - Stores 4 bytes back to row+4
// This causes an out-of-bounds access when only 4 bytes remain (istop == 4).
void png_read_filter_row_sub4_mmi(png_row_info *row_info, png_byte *row, const png_byte *prev)
{
    int istop = (int)row_info->rowbytes;

    // Emulate the assembly loop. Each iteration touches 8 bytes starting at `row`.
    // When rowbytes == 4, the very first iteration performs an OOB read/write at row+4.
    while (istop > 0) {
        uint32_t pp = 0, rp = 0;
        // Load 4 bytes at row (in-bounds when istop >= 4)
        memcpy(&pp, row + 0, sizeof(pp));
        // Load 4 bytes at row+4 (OOB when istop == 4)
        memcpy(&rp, row + 4, sizeof(rp));
        // Dummy operation analogous to paddb in the asm (not important for triggering)
        rp = rp + pp;
        // Store 4 bytes at row+4 (OOB when istop == 4)
        memcpy(row + 4, &rp, sizeof(rp));

        row   += 4;   // daddiu %[row], %[row], 0x04
        istop -= 4;   // daddiu %[istop], %[istop], -0x04
    }

    (void)prev; // Unused in this path
}

int main(void)
{
    // One pixel, 4 bytes per pixel (e.g., RGBA). This makes rowbytes == 4,
    // which triggers the bug on the first loop iteration.
    const size_t rowbytes = 4;

    png_row_info info;
    info.width = 1;      // 1 pixel
    info.rowbytes = rowbytes; // 4 bytes total in the row buffer

    // Allocate exactly 4 bytes on the heap so that any access at row+4 overflows.
    png_byte *row = (png_byte *)malloc(rowbytes);
    if (!row) {
        perror("malloc");
        return 1;
    }

    // Initialize the valid 4-byte row buffer
    for (size_t i = 0; i < rowbytes; ++i) row[i] = (png_byte)i;

    // Call the vulnerable function: this will attempt to access row+4
    // (read then write), overflowing the 4-byte allocation.
    png_read_filter_row_sub4_mmi(&info, row, NULL);

    // Cleanup (won't be reached if ASan aborts on detected overflow)
    free(row);
    return 0;
}
