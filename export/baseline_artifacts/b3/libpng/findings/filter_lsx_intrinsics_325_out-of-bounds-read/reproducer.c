// Standalone C reproducer for OOB read in png_read_filter_row_paeth3_lsx
// The real bug: __lsx_vldrepl_w(nxt, 0) unconditionally performs a 4-byte load
// even when rowbytes == 3, causing a 1-byte out-of-bounds read.

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>

// Minimal libpng-like types
typedef unsigned char png_byte;
typedef struct {
    size_t rowbytes;
} png_row_info;

// Minimal placeholder for LSX vector type
typedef struct { uint32_t lane[4]; } __m128i;

// Volatile sink to prevent dead-code elimination
static volatile __m128i sink_vec;

// Stub of LSX intrinsic: replicate 32-bit word load
// This intentionally performs an unguarded 4-byte load from p,
// reproducing the same out-of-bounds read behavior as the real intrinsic
// when called with a 3-byte row buffer.
static __attribute__((noinline)) __m128i __lsx_vldrepl_w(const png_byte *p, int imm)
{
    (void)imm; // unused
    __m128i v;
    // Unconditional 4-byte load (will read 1 byte past a 3-byte buffer)
    uint32_t x = *(const uint32_t *)p;  // ASan will flag this when p points to 3-byte allocation
    v.lane[0] = v.lane[1] = v.lane[2] = v.lane[3] = x;
    return v;
}

// Stub of LSX byte replicate (not strictly needed for the crash path)
static __attribute__((noinline)) __m128i __lsx_vldrepl_b(const png_byte *p, int imm)
{
    (void)imm; (void)p; __m128i v = {0}; return v;
}

// Vulnerable function (reduced to the minimal crashing path)
// Mirrors the beginning of loongarch/filter_lsx_intrinsics.c:png_read_filter_row_paeth3_lsx
static __attribute__((noinline)) void png_read_filter_row_paeth3_lsx(
    png_row_info *row_info, png_byte *row, const png_byte *prev_row)
{
    size_t n = row_info->rowbytes;
    png_byte *nxt = row;
    const png_byte *prev_nxt = prev_row;

    // The bug triggers here when n == 3: 4-byte loads from 3-byte buffers
    __m128i vec_a = __lsx_vldrepl_w(nxt, 0);       // OOB read of row by 1 byte
    __m128i vec_b = __lsx_vldrepl_w(prev_nxt, 0);  // Also OOB read of prev_row by 1 byte

    // Use results to avoid being optimized out
    sink_vec = vec_a;
    sink_vec = vec_b;

    (void)n;
}

int main(void)
{
    // Allocate exactly 3 bytes for current and previous rows
    size_t rowbytes = 3;
    png_byte *row = (png_byte *)malloc(rowbytes);
    png_byte *prev_row = (png_byte *)malloc(rowbytes);
    if (!row || !prev_row) {
        fprintf(stderr, "Allocation failed\n");
        return 1;
    }

    // Initialize buffers with some data
    for (size_t i = 0; i < rowbytes; ++i) {
        row[i] = (png_byte)(0x10 + i);
        prev_row[i] = (png_byte)(0x20 + i);
    }

    png_row_info info;
    info.rowbytes = rowbytes;  // 3 bytes exactly, the vulnerable edge case

    // This call triggers the out-of-bounds read due to the 4-byte load
    png_read_filter_row_paeth3_lsx(&info, row, prev_row);

    // Clean up (may not be reached if ASan aborts on error)
    free(row);
    free(prev_row);

    return 0;
}
