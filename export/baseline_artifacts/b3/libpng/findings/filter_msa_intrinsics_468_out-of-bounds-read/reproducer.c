#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

// Minimal re-declarations to match the vulnerable function's signature
typedef unsigned char png_byte;
typedef struct {
    size_t rowbytes;
} png_row_info;

// Simple 16-byte vector type to simulate MSA v16u8
typedef struct {
    uint8_t b[16];
} v16u8;

// Simulate LW (load 32-bit) used by the function
static inline int32_t LW(const png_byte *p) {
    int32_t v;
    memcpy(&v, p, 4); // safe for the chosen test case
    return v;
}

// Simulate MSA 16-byte load: always reads 16 bytes from memory
// This is the source of the out-of-bounds read when fewer than 16 bytes remain.
static inline v16u8 LD_UB(const png_byte *p) {
    v16u8 r;
    // Intentionally read 16 bytes regardless of remaining length
    for (int i = 0; i < 16; i++) r.b[i] = p[i];
    return r;
}

// Simulate a 16-byte store used later in the function
static inline void ST_UB(v16u8 v, png_byte *p) {
    for (int i = 0; i < 16; i++) p[i] = v.b[i];
}

// Helper to prepare initial vector state; content is irrelevant for triggering the bug
static inline v16u8 insert_w_zero(int32_t w) {
    v16u8 z;
    memset(&z, 0, sizeof z);
    memcpy(z.b, &w, 4);
    return z;
}

// Reimplementation of the vulnerable code path, simplified but preserving the faulty 16-byte load on the tail
static void png_read_filter_row_sub4_msa(png_row_info *row_info, png_byte *row, const png_byte *prev_row) {
    (void)prev_row; // unused in this reproducer

    size_t count;
    size_t istop = row_info->rowbytes;
    png_byte *src = row;
    png_byte *nxt = row + 4;
    int32_t inp0;
    v16u8 src0, src1, dst0;

    // As in the original code: process in 16-byte chunks after the first 4 bytes
    istop -= 4;

    // Initial 4-byte load (safe for our chosen input size)
    inp0 = LW(src);
    src += 4;
    src0 = insert_w_zero(inp0);

    // Vulnerable loop: performs 16-byte loads unconditionally
    for (count = 0; count < istop; count += 16) {
        // Out-of-bounds read happens here when fewer than 16 bytes remain
        src1 = LD_UB(src); // <-- ASan should flag this read when rowbytes-4 < 16
        src += 16;

        // Dummy ops and store to keep side effects (can also trigger OOB write, but read occurs first)
        dst0 = src1;
        ST_UB(dst0, nxt);
        nxt += 16;
    }
}

int main(void) {
    // Choose a row size that makes (rowbytes - 4) = 1, so the loop runs once with only 1 byte remaining
    // This forces LD_UB to overread by 15 bytes.
    png_row_info info;
    info.rowbytes = 5; // 4 + 1 => tail < 16

    png_byte *row = (png_byte *)malloc(info.rowbytes);
    if (!row) return 1;

    // Initialize the row with some data
    for (size_t i = 0; i < info.rowbytes; i++) row[i] = (png_byte)(i + 1);

    // Call the vulnerable function; ASan should report an out-of-bounds read in LD_UB
    png_read_filter_row_sub4_msa(&info, row, NULL);

    free(row);
    return 0;
}
