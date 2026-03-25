#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Minimal type/const redeclarations matching libpng style */
typedef uint8_t  png_byte;
typedef uint16_t png_uint_16;
typedef png_byte* png_bytep;

#define PNG_COLOR_TYPE_RGB_ALPHA 6

typedef struct png_row_info_s {
    unsigned int width;      /* number of pixels in row */
    int bit_depth;           /* 8 or 16 in our minimal repro */
    int color_type;          /* PNG_COLOR_TYPE_* */
} png_row_info;

/* Global gamma tables as used by the vulnerable code */
static png_uint_16 gamma_16_table_storage[256][256];
static png_uint_16* gamma_16_table[256];
static int gamma_shift = 0; /* keep 0 so index uses full high byte */

/* Vulnerable function fragment reconstructed from pngrtran.c */
__attribute__((noinline))
static void png_do_gamma(png_row_info *row_info, png_bytep row)
{
    png_bytep sp;
    unsigned int i;

    if (row_info->color_type == PNG_COLOR_TYPE_RGB_ALPHA) {
        if (row_info->bit_depth == 8) {
            /* not used in this repro */
        } else { /* 16-bit path */
            unsigned int row_width = row_info->width;
            sp = row;
            for (i = 0; i < row_width; i++) {
                /* R (2 bytes) */
                png_uint_16 v = gamma_16_table[*(sp + 1) >> gamma_shift][*sp];
                *sp = (png_byte)((v >> 8) & 0xff);
                *(sp + 1) = (png_byte)(v & 0xff);
                sp += 2;

                /* G (2 bytes) */
                v = gamma_16_table[*(sp + 1) >> gamma_shift][*sp];
                *sp = (png_byte)((v >> 8) & 0xff);
                *(sp + 1) = (png_byte)(v & 0xff);
                sp += 2;

                /* B (2 bytes) */
                v = gamma_16_table[*(sp + 1) >> gamma_shift][*sp];
                *sp = (png_byte)((v >> 8) & 0xff);
                *(sp + 1) = (png_byte)(v & 0xff);
                /* BUG: skips 4 bytes (advances by 10 bytes per pixel instead of 8) */
                sp += 4; /* should be 2 to skip 2-byte alpha */
            }
        }
    }
}

int main(void)
{
    /* Initialize gamma_16_table to an identity-like mapping */
    for (int i = 0; i < 256; i++) {
        gamma_16_table[i] = &gamma_16_table_storage[i][0];
        for (int j = 0; j < 256; j++) {
            gamma_16_table_storage[i][j] = (png_uint_16)((i << 8) | j);
        }
    }

    /* Set up a 16-bit RGBA row with 3 pixels (8 bytes/pixel = 24 bytes) */
    const unsigned int row_width = 3; /* minimal value to trigger the overflow */
    const size_t bytes_per_pixel = 8; /* R(2) G(2) B(2) A(2) */
    const size_t row_bytes = row_width * bytes_per_pixel; /* 24 bytes */

    png_bytep row = (png_bytep)malloc(row_bytes);
    if (!row) {
        perror("malloc");
        return 1;
    }

    /* Fill the row with deterministic nonzero data */
    for (size_t i = 0; i < row_bytes; i++) {
        row[i] = (png_byte)(i & 0xFF);
    }

    png_row_info info;
    info.width = row_width;
    info.bit_depth = 16;
    info.color_type = PNG_COLOR_TYPE_RGB_ALPHA;

    /* This call will read/write past the end of 'row' on the 3rd iteration
       because the loop advances sp by 10 bytes per pixel (due to sp += 4)
       while the actual pixel size is 8 bytes. With 3 pixels, the B-channel
       of the 3rd pixel touches indices 24 and 25 (OOB; valid is 0..23). */
    png_do_gamma(&info, row);

    /* If ASan didn't abort yet, print something to keep side effects */
    printf("Completed png_do_gamma (ASan should report heap-buffer-overflow)\n");

    free(row);
    return 0;
}
