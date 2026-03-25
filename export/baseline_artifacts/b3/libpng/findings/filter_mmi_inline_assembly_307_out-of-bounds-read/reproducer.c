#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Minimal re-declarations to match the libpng-style API used by the MMI function */
typedef unsigned char png_byte;
typedef png_byte* png_bytep;
typedef const png_byte* png_const_bytep;

typedef struct png_row_info_struct {
    size_t rowbytes;
} png_row_info, *png_row_infop;

/*
 * Stand-in for mips/filter_mmi_inline_assembly.c:png_read_filter_row_paeth3_mmi
 * The real bug: processes 12 bytes per iteration but performs two 8-byte loads
 * (total 16 bytes) from row and prev at offsets 0 and +8. When rowbytes is not
 * divisible by 12, the final iteration overreads.
 */
void png_read_filter_row_paeth3_mmi(png_row_infop row_info, png_bytep row, png_const_bytep prev)
{
    size_t istop = row_info->rowbytes;
    volatile unsigned long long sink = 0; /* prevent optimization */

    for (size_t i = 0; i < istop; i += 12) {
        /* Emulate the two 8-byte loads from row and prev at +0 and +8 */
        unsigned long long rp = 0, rp1 = 0, pp = 0, pp1 = 0;
        /* First 8-byte loads (safe if i + 8 <= istop) */
        memcpy((void*)&rp,  row + i + 0, 8);
        memcpy((void*)&pp,  prev + i + 0, 8);
        /* Second 8-byte loads at +8 (this is the overread on the last iteration
           when istop % 12 != 0, mirroring the bug in the MMI version) */
        memcpy((void*)&rp1, row + i + 8, 8);
        memcpy((void*)&pp1, prev + i + 8, 8);

        /* Dummy computation to keep the loads alive */
        sink ^= rp ^ rp1 ^ pp ^ pp1;

        /* Pretend we only "process" 12 bytes, as the vector loop does */
        size_t to_process = (istop - i >= 12) ? 12 : (istop - i);
        for (size_t j = 0; j < to_process; ++j) {
            row[i + j] ^= 0xAA;
        }
    }

    if (sink == 0xDEADBEEFDEADBEEFULL) {
        /* unreachable, prevents optimizing away */
        fprintf(stderr, "sink: %llu\n", (unsigned long long)sink);
    }
}

int main(void)
{
    /* Choose a rowbytes value not divisible by 12 (e.g., 27 = 3 bytes/pixel * 9 pixels),
       so the final iteration has a tail < 12 and the second 8-byte load overreads. */
    const size_t rowbytes = 27; /* 27 % 12 = 3 -> triggers overread on last iter */

    png_row_info info;
    info.rowbytes = rowbytes;

    png_bytep row  = (png_bytep)malloc(rowbytes);
    png_bytep prev = (png_bytep)malloc(rowbytes);

    if (!row || !prev) {
        fprintf(stderr, "Allocation failed\n");
        return 1;
    }

    /* Initialize buffers with known data */
    for (size_t i = 0; i < rowbytes; ++i) {
        row[i]  = (png_byte)(i & 0xFF);
        prev[i] = (png_byte)(0x80 | (i & 0x7F));
    }

    /* This call should trigger an ASan out-of-bounds read on the last iteration */
    png_read_filter_row_paeth3_mmi(&info, row, prev);

    /* Clean up (ASan reports appear before program exit) */
    free(row);
    free(prev);

    return 0;
}
