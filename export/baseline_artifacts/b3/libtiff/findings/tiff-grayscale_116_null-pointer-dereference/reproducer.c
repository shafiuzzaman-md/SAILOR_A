#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <math.h>

/*
   Stub out the libtiff public API used by contrib/dbs/tiff-grayscale.c so we don't
   depend on real file I/O. Our stubs allow reaching the vulnerable code path.
*/
typedef struct TIFF_dummy { int dummy; } TIFF;

/* Define dummy tag and enum values used by TIFFSetField in the original code */
#define TIFFTAG_IMAGEWIDTH            256
#define TIFFTAG_IMAGELENGTH           257
#define TIFFTAG_BITSPERSAMPLE         258
#define TIFFTAG_COMPRESSION           259
#define TIFFTAG_PHOTOMETRIC           262
#define TIFFTAG_SAMPLESPERPIXEL       277
#define TIFFTAG_ROWSPERSTRIP          278
#define TIFFTAG_PLANARCONFIG          284
#define TIFFTAG_REFERENCEBLACKWHITE   532
#define TIFFTAG_TRANSFERFUNCTION      301
#define TIFFTAG_RESOLUTIONUNIT        296

#define COMPRESSION_NONE              1
#define PHOTOMETRIC_MINISBLACK        1
#define PLANARCONFIG_CONTIG           1
#define RESUNIT_NONE                  1

/* Stubs: succeed but do nothing */
TIFF *TIFFOpen(const char *name, const char *mode) {
    (void)name; (void)mode;
    /* Return non-NULL to follow the vulnerable path */
    static TIFF t; return &t;
}
int TIFFSetField(TIFF *tif, uint32_t tag, ...) {
    (void)tif; (void)tag;
    va_list ap; va_start(ap, tag); va_end(ap);
    return 1;
}
int TIFFWriteScanline(TIFF *tif, void *buf, uint32_t row, uint16_t sample) {
    (void)tif; (void)buf; (void)row; (void)sample;
    return 1;
}
void TIFFClose(TIFF *tif) { (void)tif; }

/* This helper mimics a failing malloc for the scan_line buffer only */
static void *failing_scanline_malloc(size_t size) {
    (void)size;
    return NULL; /* Simulate OOM specifically for scan_line */
}

int main(void) {
    /* Minimal setup mirroring contrib/dbs/tiff-grayscale.c */
    const int WIDTH = 16;
    const int HEIGHT = 2;
    int bits_per_pixel = 8; /* Choose 8bpp to hit the case at original line 116 */

    /* Parameters driving gray table computation and pixel selection */
    int nchunks = 2;            /* Keep >1 so cmsize-1 != 0 */
    int chunk_size = 1;         /* Simple chunking so divisions are safe */
    int cmsize = nchunks * nchunks;

    uint16_t *gray = (uint16_t *)malloc((size_t)cmsize * sizeof(uint16_t));
    if (!gray) {
        fprintf(stderr, "unexpected: gray allocation failed\n");
        return 1;
    }

    gray[0] = 3000;
    for (int i = 1; i < cmsize; i++) {
        gray[i] = (uint16_t)(-log10((double)i / (cmsize - 1)) * 1000.0);
    }

    float refblackwhite[2];
    refblackwhite[0] = 0.0f;
    refblackwhite[1] = (float)((1U << bits_per_pixel) - 1U);

    TIFF *tif = TIFFOpen("dummy.tif", "w");
    if (tif == NULL) {
        fprintf(stderr, "can't open dummy.tif as a TIFF file\n");
        free(gray);
        return 0;
    }

    TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, WIDTH);
    TIFFSetField(tif, TIFFTAG_IMAGELENGTH, HEIGHT);
    TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, bits_per_pixel);
    TIFFSetField(tif, TIFFTAG_COMPRESSION, COMPRESSION_NONE);
    TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
    TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, 1);
    TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, 1);
    TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
    TIFFSetField(tif, TIFFTAG_REFERENCEBLACKWHITE, refblackwhite);
    TIFFSetField(tif, TIFFTAG_TRANSFERFUNCTION, gray);
    TIFFSetField(tif, TIFFTAG_RESOLUTIONUNIT, RESUNIT_NONE);

    /* The vulnerable allocation: simulate malloc failure for scan_line */
    unsigned char *scan_line = (unsigned char *)failing_scanline_malloc((size_t)WIDTH / (8 / bits_per_pixel));

    /* Proceed as in the original code without checking scan_line for NULL */
    for (int i = 0; i < HEIGHT; i++) {
        int k = 0;
        for (int j = 0; j < WIDTH;) {
            int gray_index = (j / chunk_size) + ((i / chunk_size) * nchunks);
            switch (bits_per_pixel) {
                case 8:
                    /* NULL-pointer-dereference occurs here */
                    scan_line[k++] = (unsigned char)gray_index;
                    j++;
                    break;
                case 4:
                    scan_line[k++] = (unsigned char)((gray_index << 4) + gray_index);
                    j += 2;
                    break;
                case 2:
                    scan_line[k++] = (unsigned char)((gray_index << 6) + (gray_index << 4) + (gray_index << 2) + gray_index);
                    j += 4;
                    break;
                default:
                    break;
            }
        }
        TIFFWriteScanline(tif, scan_line, (uint32_t)i, 0);
    }

    /* Never reached due to crash, but keep for completeness */
    free(scan_line);
    TIFFClose(tif);
    free(gray);
    return 0;
}
