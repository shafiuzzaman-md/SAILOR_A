#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/*
 Self-contained reproducer for the integer-overflow-to-heap-buffer-overflow in
 contrib/dbs/xtiff/xtiff.c:GetTIFFImage at the allocation:
     imageMemory = (char *)malloc(tfImageWidth * tfImageHeight * 4);

 The overflow happens because the allocation size is computed in 32-bit
 arithmetic, but later code writes 4 bytes per pixel for tfImageWidth * tfImageHeight
 pixels, overflowing the (too small) allocation.
*/

/* Global variables mimicking the ones used by xtiff.c */
static uint32_t tfImageWidth;
static uint32_t tfImageHeight;
static int tfImageDepth; /* used to select the 24/32 bpp branch */
static size_t tfBytesPerRow;
static char *imageMemory;

/* No-op checker macro to keep the code flow similar to original */
#define MCHECK(p) do { (void)(p); } while (0)

/* This is a minimal, self-contained version of GetTIFFImage() focusing on the
 * vulnerable allocation and subsequent pixel writes. */
static void GetTIFFImage(void)
{
    char *scan_line, *output_p;

    /* Stub scanline allocation (value doesn't matter for triggering the bug) */
    tfBytesPerRow = 1;
    scan_line = (char *)malloc(tfBytesPerRow);
    MCHECK(scan_line);

    if ((tfImageDepth == 32) || (tfImageDepth == 24))
    {
        /*
         * Vulnerable allocation: computed in 32-bit and then passed to malloc.
         * We purposely choose tfImageWidth and tfImageHeight so that this
         * overflows 32-bit to a very small positive number.
         */
        uint32_t alloc32 = (uint32_t)(tfImageWidth * tfImageHeight * 4u);
        output_p = imageMemory = (char *)malloc(alloc32);
        MCHECK(imageMemory);

        /* Print the (overflowed) allocation size for clarity */
        fprintf(stderr, "Requested pixels: %u x %u (depth %d) => 32-bit alloc size: %u bytes\n",
                tfImageWidth, tfImageHeight, tfImageDepth, alloc32);

        /*
         * The real code would write 4 bytes per pixel for tfImageWidth * tfImageHeight
         * pixels. To demonstrate the overflow quickly (without iterating billions of
         * pixels), we write a small fixed number of pixels (8 => 32 bytes), which
         * exceeds the tiny overflowed allocation (16 bytes in our crafted case),
         * triggering an ASan heap-buffer-overflow.
         */
        size_t pixels_to_write = 8; /* 8 pixels * 4 bytes/pixel = 32 bytes */
        for (size_t p = 0; p < pixels_to_write; ++p)
        {
            size_t idx = p * 4;
            /* Four bytes per pixel */
            output_p[idx + 0] = 0x11;
            output_p[idx + 1] = 0x22;
            output_p[idx + 2] = 0x33;
            output_p[idx + 3] = 0x44;
        }

        /* Clean up to silence potential leak reports after ASan abort */
        free(scan_line);
        free(imageMemory);
    }
}

int main(void)
{
    /*
     * Choose dimensions so that (tfImageWidth * tfImageHeight * 4) overflows 32-bit
     * to a small positive value. We pick:
     *   tfImageWidth  = 2^30 + 4 = 1,073,741,828
     *   tfImageHeight = 1
     * Then: width * height * 4 = (2^30 + 4) * 4 = 2^32 + 16, which wraps to 16 (uint32_t).
     */
    tfImageDepth  = 24;              /* enter the vulnerable 24/32 bpp branch */
    tfImageWidth  = 1073741828u;     /* 2^30 + 4 */
    tfImageHeight = 1u;

    GetTIFFImage();
    return 0;
}
