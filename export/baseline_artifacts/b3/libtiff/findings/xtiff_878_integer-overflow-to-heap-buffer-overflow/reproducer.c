#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/*
   Standalone reproducer for the integer-overflow-to-heap-buffer-overflow in
   contrib/dbs/xtiff/xtiff.c:GetTIFFImage (line ~878).

   We reimplement just the vulnerable branch:
     - xImageDepth == 8 and tfImageDepth == 2
     - allocation: malloc(tfBytesPerRow * 4 * tfImageHeight + 4)
     - unpack loop writes ~4 * ceil(tfImageWidth/4) bytes per row

   We choose values such that the 32-bit int multiplication overflows to a
   small positive number (4), but the unpack loop writes 1024 bytes on the
   first row, overflowing the undersized heap buffer immediately.
*/

typedef struct { int dummy; } TIFF; /* Dummy type; we don't actually use libtiff here */

/* Make our local TIFFReadScanline to avoid linking to the real one. */
static int TIFFReadScanline(TIFF *tfFile, void *buf, unsigned int row, int sample)
{
    (void)tfFile; (void)sample;
    /* Succeed only on the first row; then return <0 to break outer loop early */
    if (row == 0) {
        /* Fill with a pattern; the caller allocated at least tfBytesPerRow bytes */
        memset(buf, 0xAA, 256); /* matches tfBytesPerRow we set below */
        return 1; /* success */
    }
    return -1; /* stop after the first row */
}

#define MCHECK(p) do { if (!(p)) { fprintf(stderr, "malloc failed\n"); exit(1); } } while (0)

int main(void)
{
    /* Force execution into the vulnerable branch (xImageDepth==8 && tfImageDepth==2) */
    int xImageDepth = 8;
    int tfImageDepth = 2;

    /* Image geometry crafted for the overflow and subsequent OOB write */
    int tfImageWidth  = 1024;      /* number of pixels per row */
    int tfBytesPerRow = (tfImageWidth + 3) / 4; /* ceil(W/4) for 2-bit samples => 256 */

    /* Choose height so that: tfBytesPerRow * 4 * tfImageHeight == 2^32
       -> overflows 32-bit signed int to 0, then +4 => 4-byte allocation. */
    int tfImageHeight = 4194304;   /* 4,194,304 = 2^32 / (tfBytesPerRow * 4) = 2^32 / 1024 */

    char *imageMemory = NULL;
    char *output_p = NULL;
    const int basePixel = 0;

    /* Temporary input buffer for a scanline (packed 2-bpp data), size = tfBytesPerRow */
    unsigned char *scan_line = (unsigned char *)malloc((size_t)tfBytesPerRow);
    MCHECK(scan_line);

    /* --- Vulnerable allocation from xtiff.c:878 --- */
    if ((xImageDepth == 8) && (tfImageDepth == 2)) {
        /* Intentional 32-bit signed overflow here: 256 * 4 * 4,194,304 = 4,294,967,296 -> wraps to 0 */
        int alloc_size = tfBytesPerRow * 4 * tfImageHeight + 4;
        fprintf(stderr, "Computed alloc_size (int) = %d (expected 4 if overflowed)\n", alloc_size);
        imageMemory = (char *)malloc((size_t)alloc_size);
        MCHECK(imageMemory);

        TIFF fake;
        TIFF *tfFile = &fake;

        int i, j;
        unsigned char *input_p;

        for (i = 0; i < tfImageHeight; i++) {
            if (TIFFReadScanline(tfFile, scan_line, (unsigned int)i, 0) < 0)
                break; /* break after first row to keep the reproducer fast */

            /* Start of row i output */
            output_p = &imageMemory[i * tfImageWidth];
            input_p = scan_line;

            /* This writes 4 bytes per 4 pixels, i.e., ~4 * ceil(W/4) bytes per row.
               For W=1024, it writes exactly 1024 bytes in the first row.
               But we only allocated 4 bytes due to overflow, so ASan will report HBO. */
            for (j = 0; j < tfImageWidth; j += 4, input_p++) {
                *output_p++ = (char)((*input_p >> 6) + basePixel);
                *output_p++ = (char)(((*input_p >> 4) & 3) + basePixel);
                *output_p++ = (char)(((*input_p >> 2) & 3) + basePixel);
                *output_p++ = (char)((*input_p & 3) + basePixel);
            }
        }
    } else {
        fprintf(stderr, "Did not enter vulnerable branch; check depth setup.\n");
    }

    /* Clean up (we likely never reach here due to ASan abort on overflow) */
    free(scan_line);
    free(imageMemory);
    return 0;
}
