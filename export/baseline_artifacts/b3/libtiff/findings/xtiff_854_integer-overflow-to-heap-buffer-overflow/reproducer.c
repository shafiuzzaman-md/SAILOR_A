#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <limits.h>

/*
 * Standalone reproducer for the integer-overflow-to-heap-buffer-overflow in
 * contrib/dbs/xtiff/xtiff.c:GetTIFFImage at the allocation:
 *   malloc(tfBytesPerRow * 2 * tfImageHeight + 2)
 * followed by per-row expansion writing ~2 * ceil(tfImageWidth/2) bytes.
 *
 * We reimplement just the vulnerable branch ((xImageDepth==8)&&(tfImageDepth==4)),
 * set tfBytesPerRow to force the allocation to wrap to a small size, and set a
 * large tfImageWidth to cause a large write that overflows the undersized buffer.
 */

#define MCHECK(p) do { if (!(p)) { fprintf(stderr, "alloc failed\n"); exit(1);} } while (0)

static size_t tfBytesPerRow;
static size_t tfImageHeight, tfImageWidth;
static int tfImageDepth, xImageDepth;
static unsigned char basePixel = 0;
static unsigned char *scan_line;
static size_t g_scanline_size;

/* Stub TIFFReadScanline: fill the provided buffer with dummy data */
static int TIFFReadScanline(void *tif, void *buf, unsigned int row, unsigned short s)
{
    (void)tif; (void)row; (void)s;
    memset(buf, 0xFF, g_scanline_size);
    return 1; /* success */
}

static void GetTIFFImage_trigger(void)
{
    char *imageMemory;
    char *output_p;
    unsigned char *input_p;
    size_t i, j;

    if ((xImageDepth == 8) && (tfImageDepth == 4))
    {
        /* Vulnerable allocation (matches the upstream code pattern) */
        output_p = imageMemory = (char *)malloc(tfBytesPerRow * 2 * tfImageHeight + 2);
        MCHECK(imageMemory);

        for (i = 0; i < tfImageHeight; i++)
        {
            if (TIFFReadScanline(NULL, scan_line, (unsigned int)i, 0) < 0)
                break;
            output_p = &imageMemory[i * tfImageWidth];
            input_p = scan_line;
            for (j = 0; j < tfImageWidth; j += 2, input_p++)
            {
                /* 4-bit to 8-bit expansion writes 2 bytes per input byte */
                *output_p++ = (unsigned char)((*input_p >> 4) + basePixel);
                *output_p++ = (unsigned char)((*input_p & 0x0F) + basePixel);
            }
        }

        /* Prevent compiler from optimizing away writes */
        if (imageMemory) {
            volatile unsigned char sink = (unsigned char)imageMemory[0];
            (void)sink;
        }
    }
}

int main(void)
{
    /* Choose parameters to force integer overflow in allocation:
     * size = tfBytesPerRow * 2 * tfImageHeight + 2
     * Pick tfBytesPerRow so that (tfBytesPerRow * 2) wraps to a small value.
     */
    tfImageWidth  = (size_t)1 << 20;  /* 1,048,576 pixels per row (even) */
    tfImageHeight = 2;                /* two rows */
    tfImageDepth  = 4;                /* source depth triggers the branch */
    xImageDepth   = 8;                /* destination depth triggers the branch */

    /* Cause wrap: (SIZE_MAX/2 + 100) * 2 -> SIZE_MAX + 200 -> wraps to 199
     * Then 199 * height (2) + 2 => 400-byte allocation, far too small. */
    tfBytesPerRow = (SIZE_MAX / 2) + 100;

    /* Allocate a scanline buffer large enough for the read loop: ceil(width/2) */
    g_scanline_size = (tfImageWidth + 1) / 2;
    scan_line = (unsigned char *)malloc(g_scanline_size);
    MCHECK(scan_line);
    memset(scan_line, 0xAB, g_scanline_size);

    /* Trigger the vulnerable logic */
    GetTIFFImage_trigger();

    free(scan_line);
    return 0;
}
