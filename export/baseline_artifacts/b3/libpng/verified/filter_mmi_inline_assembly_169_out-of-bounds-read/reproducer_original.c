#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// Minimal re-declarations to match the vulnerable function's signature
typedef unsigned char png_byte;
typedef struct png_row_info_s {
    size_t rowbytes;
} png_row_info;

// A stub implementation that mirrors the vulnerable access pattern:
// - Iterates in 12-byte steps (like the MMI asm path)
// - Performs two 8-byte reads from both 'row' and 'prev' at offsets 0 and 8
// This reproduces the out-of-bounds read when rowbytes is not a multiple of 12.
__attribute__((noinline))
void png_read_filter_row_avg3_mmi(png_row_info *row_info, png_byte *row, const png_byte *prev)
{
    int istop = (int)row_info->rowbytes;
    size_t off = 0;
    volatile unsigned long long sink = 0; // keep reads from being optimized out

    while (istop > 0) {
        unsigned long long rp, rp1, pp, pp1;
        // These four 8-byte reads emulate the asm gsldrc1/gsldlc1 pairs at 0x00 and 0x08
        // Reading 16 bytes total from 'row' and 16 from 'prev' per 12-byte step.
        memcpy((void *)&rp,  row + off,      8);        // may read past end
        memcpy((void *)&pp,  prev + off,     8);        // may read past end
        memcpy((void *)&rp1, row + off + 8,  8);        // may read past end
        memcpy((void *)&pp1, prev + off + 8, 8);        // may read past end
        sink ^= rp ^ rp1 ^ pp ^ pp1;

        off   += 12;
        istop -= 12;
    }

    // Use sink so compiler keeps the above loads
    if (sink == 0xDEADBEEFDEADBEEFULL) {
        fprintf(stderr, "unlikely\n");
    }
}

int main(void)
{
    // Choose a rowbytes value that is NOT a multiple of 12.
    // For example, 3 bytes/pixel * 5 pixels = 15 bytes (common for RGB)
    // The first loop iteration will read 16 bytes (0..15) from buffers of size 15,
    // causing a 1-byte out-of-bounds read; subsequent iterations read even further OOB.
    const size_t rowbytes = 15; // not a multiple of 12

    png_byte *row  = (png_byte *)malloc(rowbytes);
    png_byte *prev = (png_byte *)malloc(rowbytes);
    if (!row || !prev) {
        fprintf(stderr, "Allocation failed\n");
        return 1;
    }

    memset(row,  0xAA, rowbytes);
    memset(prev, 0xBB, rowbytes);

    png_row_info info;
    info.rowbytes = rowbytes;

    // This call should trigger ASan OOB-read due to the 16-byte loads per 12-byte step
    png_read_filter_row_avg3_mmi(&info, row, prev);

    free(row);
    free(prev);
    return 0;
}
