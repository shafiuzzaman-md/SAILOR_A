#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <tiffio.h>

int main(void) {
    const char *filename = "repro_jpegtables_oob.tif";

    TIFF *tif = TIFFOpen(filename, "w");
    if (!tif) {
        fprintf(stderr, "Failed to open TIFF for writing\n");
        return 1;
    }

    uint32_t width = 16;
    uint32_t height = 16;

    /* Basic required tags for a simple JPEG-compressed TIFF */
    if (!TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, width)) {
        fprintf(stderr, "TIFFSetField IMAGEWIDTH failed\n");
        return 1;
    }
    if (!TIFFSetField(tif, TIFFTAG_IMAGELENGTH, height)) {
        fprintf(stderr, "TIFFSetField IMAGELENGTH failed\n");
        return 1;
    }
    if (!TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, 8)) {
        fprintf(stderr, "TIFFSetField BITSPERSAMPLE failed\n");
        return 1;
    }
    if (!TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, 3)) {
        fprintf(stderr, "TIFFSetField SAMPLESPERPIXEL failed\n");
        return 1;
    }
    if (!TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB)) {
        fprintf(stderr, "TIFFSetField PHOTOMETRIC failed\n");
        return 1;
    }
    if (!TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG)) {
        fprintf(stderr, "TIFFSetField PLANARCONFIG failed\n");
        return 1;
    }
    /* Use JPEG compression */
    if (!TIFFSetField(tif, TIFFTAG_COMPRESSION, COMPRESSION_JPEG)) {
        fprintf(stderr, "TIFFSetField COMPRESSION=JPEG failed (libjpeg not available?)\n");
        return 1;
    }

    /* Make RowsPerStrip equal to image length to bypass height multiple checks */
    if (!TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, height)) {
        fprintf(stderr, "TIFFSetField ROWSPERSTRIP failed\n");
        return 1;
    }

    /* Supply an application-provided JPEGTables buffer that is shorter than 8 bytes. */
    static const uint8_t short_jt[4] = { 0, 0, 0, 0 };
    if (!TIFFSetField(tif, TIFFTAG_JPEGTABLES, (uint32_t)sizeof(short_jt), short_jt)) {
        fprintf(stderr, "TIFFSetField JPEGTABLES failed\n");
        return 1;
    }

    /* Trigger encoding, which will call JPEGSetupEncode internally. */
    const tsize_t scanline_size = TIFFScanlineSize(tif);
    uint8_t *scanline = (uint8_t *)_TIFFmalloc(scanline_size);
    if (!scanline) {
        fprintf(stderr, "Failed to allocate scanline buffer\n");
        return 1;
    }
    memset(scanline, 0xAA, scanline_size);

    /* The first write triggers codec setup and hits the vulnerable memcmp() */
    if (TIFFWriteScanline(tif, scanline, 0, 0) < 0) {
        fprintf(stderr, "TIFFWriteScanline failed (but setup was attempted)\n");
    }

    _TIFFfree(scanline);
    TIFFClose(tif);

    fprintf(stderr, "Reproducer completed. If built with ASan, an out-of-bounds read should have been reported.\n");
    return 0;
}
