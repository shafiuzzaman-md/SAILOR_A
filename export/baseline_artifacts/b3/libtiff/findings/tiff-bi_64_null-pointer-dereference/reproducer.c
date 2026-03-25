#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tiffio.h"

#define WIDTH 512
#define HEIGHT WIDTH

/*
 * We simulate the malloc failure that tiff-bi.c does not check for by
 * using a wrapper that returns NULL exactly for the WIDTH/8 allocation
 * performed by the sample. This reliably triggers the NULL dereference
 * on the first write to scan_line[i].
 */
static int fail_alloc_now = 0;
static void* failing_malloc(size_t size)
{
    if (fail_alloc_now && size == (WIDTH / 8))
        return NULL;
    return malloc(size);
}

int main(void)
{
    int i;
    unsigned char *scan_line;
    TIFF *tif;
    const char *out_path = "/tmp/tiff_bi_repro.tif";

    tif = TIFFOpen(out_path, "w");
    if (tif == NULL)
    {
        fprintf(stderr, "can't open %s as a TIFF file (continuing to trigger bug)\n", out_path);
        /* Even if opening fails, we continue to demonstrate the NULL deref. */
    }
    else
    {
        TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, WIDTH);
        TIFFSetField(tif, TIFFTAG_IMAGELENGTH, HEIGHT);
        TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, 1);
        TIFFSetField(tif, TIFFTAG_COMPRESSION, COMPRESSION_NONE);
        TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
        TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, 1);
        TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, 1);
        TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
        TIFFSetField(tif, TIFFTAG_RESOLUTIONUNIT, RESUNIT_NONE);
    }

    /* Enable failure for the exact allocation size used in the sample. */
    fail_alloc_now = 1;
    scan_line = (unsigned char *)failing_malloc(WIDTH / 8);

    /*
     * This loop mirrors contrib/dbs/tiff-bi.c:64 where scan_line is written
     * without checking for malloc failure. Because failing_malloc returned
     * NULL, this will dereference a NULL pointer and crash under ASan.
     */
    for (i = 0; i < (WIDTH / 8) / 2; i++)
        scan_line[i] = 0;  /* NULL pointer dereference here */

    /* The remaining code is identical in structure to the sample, but will
     * not be reached due to the crash above. It is kept for completeness. */
    for (i = (WIDTH / 8) / 2; i < (WIDTH / 8); i++)
        scan_line[i] = 255;

    for (i = 0; i < HEIGHT / 2; i++)
        TIFFWriteScanline(tif, scan_line, i, 0);

    for (i = 0; i < (WIDTH / 8) / 2; i++)
        scan_line[i] = 255;

    for (i = (WIDTH / 8) / 2; i < (WIDTH / 8); i++)
        scan_line[i] = 0;

    for (i = HEIGHT / 2; i < HEIGHT; i++)
        TIFFWriteScanline(tif, scan_line, i, 0);

    free(scan_line);
    if (tif) TIFFClose(tif);
    return 0;
}
