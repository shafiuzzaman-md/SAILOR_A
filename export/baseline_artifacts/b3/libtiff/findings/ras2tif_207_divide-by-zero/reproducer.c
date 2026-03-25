#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "libtiff/tiffio.h"

int main(void) {
    // Configure image so that bytes-per-scanline (bpsl) > 8192.
    // For depth=24 bpp, bpsl ~ 3*width. width=2731 -> bpsl=8194.
    int depth = 24;     // 24 bpp (RGB)
    int width = 2731;   // Chosen to push bpsl above 8192
    int height = 10;    // Non-zero so height/rowsperstrip triggers FPE

    int samplesperpixel = 0;
    int bitspersample = 0;
    int photometric = 0;

    switch (depth) {
        case 1:
            samplesperpixel = 1; bitspersample = 1; photometric = PHOTOMETRIC_MINISBLACK; break;
        case 8:
            samplesperpixel = 1; bitspersample = 8; photometric = PHOTOMETRIC_PALETTE; break;
        case 24:
            samplesperpixel = 3; bitspersample = 8; photometric = PHOTOMETRIC_RGB; break;
        case 32:
            samplesperpixel = 4; bitspersample = 8; photometric = PHOTOMETRIC_RGB; break;
        default:
            fprintf(stderr, "bogus depth: %d\n", depth);
            return 1;
    }

    // Replicate ras2tif.c logic
    int bpsl = ((depth * width + 15) >> 3) & ~1;  // bytes per scanline, even-aligned
    int rowsperstrip = (8 * 1024) / bpsl;         // becomes 0 when bpsl > 8192

    printf("Computed bpsl=%d, rowsperstrip=%d (expect 0 to trigger divide-by-zero)\n", bpsl, rowsperstrip);

    TIFF *tif = TIFFOpen("repro.tif", "w");
    if (!tif) {
        fprintf(stderr, "Failed to open output TIFF file.\n");
        return 1;
    }

    // Set a few fields similar to ras2tif before the vulnerable call
    TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, width);
    TIFFSetField(tif, TIFFTAG_IMAGELENGTH, height);
    TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, bitspersample);
    TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, photometric);
    TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, samplesperpixel);
    TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, rowsperstrip);

    // Vulnerable expression: height / rowsperstrip with rowsperstrip == 0
    // This reproduces the divide-by-zero at ras2tif.c:207
    TIFFSetField(tif, TIFFTAG_STRIPBYTECOUNTS, height / rowsperstrip);

    TIFFClose(tif);
    return 0;
}
