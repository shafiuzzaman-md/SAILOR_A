#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <limits.h>

/* Minimal re-declarations to mirror rpng-win.c logic without Windows headers */
typedef unsigned char uch;
#define PROGNAME "repro"

/* Real BITMAPINFOHEADER is 40 bytes; we replicate full layout to keep sizeof() == 40 */
typedef struct {
    uint32_t biSize;
    int32_t  biWidth;
    int32_t  biHeight;
    uint16_t biPlanes;
    uint16_t biBitCount;
    uint32_t biCompression;
    uint32_t biSizeImage;
    int32_t  biXPelsPerMeter;
    int32_t  biYPelsPerMeter;
    uint32_t biClrUsed;
    uint32_t biClrImportant;
} BITMAPINFOHEADER;

/* Globals analogous to the original file */
static size_t image_width;
static size_t image_height;
static size_t wimage_rowbytes;
static uch *dib;
static uch *wimage_data;
static unsigned char bg_blue = 0, bg_green = 0, bg_red = 0;

/* A minimized version of rpng_win_create_window that contains the vulnerable logic */
static int rpng_win_create_window_min(void)
{
    uch *dest;
    size_t i, j;
    BITMAPINFOHEADER *bmih;

    /* Round 3*width up to a multiple of 4 (Windows DIB row alignment) */
    wimage_rowbytes = ((3*image_width + 3UL) >> 2) << 2;

    /* Guard against integer overflow in the multiplication only (as in the bug) */
    if (image_height > ((size_t)(-1)) / wimage_rowbytes) {
        fprintf(stderr, PROGNAME ":  image_data buffer would be too large\n");
        return 4; /* fail */
    }

    /* Vulnerable allocation: addition may overflow size_t */
    dib = (uch *)malloc(sizeof(BITMAPINFOHEADER) + wimage_rowbytes * image_height);
    if (!dib) {
        fprintf(stderr, PROGNAME ": malloc failed\n");
        return 4; /* fail */
    }

    /* This write overruns the (too-small) allocation if the addition overflowed */
    memset(dib, 0, sizeof(BITMAPINFOHEADER));

    /* Continue as original (not reached if ASan detects the overflow above) */
    bmih = (BITMAPINFOHEADER *)dib;
    bmih->biSize = (uint32_t)sizeof(BITMAPINFOHEADER);
    bmih->biWidth = (int32_t)image_width;
    bmih->biHeight = -((int32_t)image_height);
    bmih->biPlanes = 1;
    bmih->biBitCount = 24;
    bmih->biCompression = 0;
    wimage_data = dib + sizeof(BITMAPINFOHEADER);

    /* Fill background to mimic the original writes into wimage_data */
    for (j = 0; j < image_height; ++j) {
        dest = wimage_data + j*wimage_rowbytes;
        for (i = image_width; i > 0; --i) {
            *dest++ = bg_blue;
            *dest++ = bg_green;
            *dest++ = bg_red;
        }
    }

    return 0;
}

int main(void)
{
    /* Craft dimensions that make:
       - multiplication wimage_rowbytes * image_height not overflow (passes guard)
       - but the subsequent addition of sizeof(BITMAPINFOHEADER) overflow
    */

    /* Make rowbytes as small as possible (4) by using width = 1 */
    image_width = 1;

    size_t header = sizeof(BITMAPINFOHEADER); /* 40 */
    size_t rowbytes = ((3*image_width + 3UL) >> 2) << 2; /* -> 4 */

    /* Choose product > SIZE_MAX - header so that product + header wraps.
       To avoid malloc(0), target an overflow to size 1: product = SIZE_MAX - header + 2.
       Ensure product <= SIZE_MAX and height is integral with rowbytes. */
    size_t target_product = SIZE_MAX - header + 2; /* guarantees overflow to 1 byte */
    image_height = (target_product + rowbytes - 1) / rowbytes; /* ceil division */

    /* Sanity prints for debugging (optional) */
    fprintf(stderr, "sizeof(BITMAPINFOHEADER)=%zu, rowbytes=%zu, height=%zu\n",
            header, rowbytes, image_height);

    /* This will trigger a heap-buffer-overflow in memset due to the wrapped allocation */
    return rpng_win_create_window_min();
}
