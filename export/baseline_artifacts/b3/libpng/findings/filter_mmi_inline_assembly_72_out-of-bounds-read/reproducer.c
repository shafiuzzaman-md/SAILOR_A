// Standalone C reproducer for OOB read in png_read_filter_row_sub3_mmi
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// Minimal typedefs/structs to match the vulnerable function signature
typedef unsigned char png_byte;
typedef struct png_row_info {
    size_t rowbytes;
} png_row_info;

// Global sink to ensure loads are not optimized out
volatile uint64_t g_sink = 0;

// C approximation of the MMI routine that exhibits the same OOB read behavior:
// - Advances row by 12 bytes per iteration
// - Reads 16 bytes per iteration (two 8-byte loads at offsets 0 and 8)
// This will overread the tail when rowbytes is not a multiple of 12.
void png_read_filter_row_sub3_mmi(png_row_info *row_info, png_byte *row, const png_byte *prev) {
    (void)prev; // not used in this path
    int istop = (int)row_info->rowbytes;

    while (istop > 0) {
        // Emulate two 64-bit loads from row+0 and row+8 as in the MMI asm
        // This causes a 16-byte read per 12-byte step, leading to tail overread.
        volatile uint64_t rp = *(const uint64_t*)(row + 0);
        volatile uint64_t pp = *(const uint64_t*)(row + 8);
        g_sink ^= rp ^ pp; // use the values so the compiler keeps the loads

        row   += 12;
        istop -= 12;
    }
}

int main(void) {
    // Choose a row size not divisible by 12 to trigger the tail overread.
    // 27 = 12*2 + 3 ensures the first full iteration is in-bounds and the
    // final iteration performs the overread (matching the described bug).
    const size_t rowbytes = 27;

    png_row_info info;
    info.rowbytes = rowbytes;

    png_byte *row = (png_byte*)malloc(rowbytes);
    if (!row) {
        perror("malloc");
        return 1;
    }

    // Fill the buffer so ASan reports an OOB READ when we overrun past the end
    memset(row, 0xA5, rowbytes);

    // prev is unused in this path; provide a dummy pointer
    png_byte *prev = (png_byte*)malloc(rowbytes);
    if (!prev) {
        perror("malloc prev");
        free(row);
        return 1;
    }
    memset(prev, 0, rowbytes);

    // This call will perform 16-byte reads per 12-byte step.
    // For rowbytes=27, the second iteration reads at offsets 12..27, overreading by 1 byte.
    png_read_filter_row_sub3_mmi(&info, row, prev);

    // Prevent optimizing away
    printf("sink=%llu\n", (unsigned long long)g_sink);

    free(prev);
    free(row);
    return 0;
}
