#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fenv.h>

/* Forward declaration of the internal struct type used by libtiff. */
typedef struct TIFFYCbCrToRGB TIFFYCbCrToRGB;

/* External function from libtiff that we are targeting. */
extern int TIFFYCbCrToRGBInit(TIFFYCbCrToRGB *ycbcr, float *luma, float *refBlackWhite);

int main(void) {
    /* Enable trapping of floating-point divide-by-zero to make the bug crash deterministically. */
    feenableexcept(FE_DIVBYZERO);

    /* Allocate a large buffer to serve as the TIFFYCbCrToRGB context plus its scratch tables.
       The actual size of TIFFYCbCrToRGB inside libtiff is unknown to us, so we over-allocate. */
    size_t buf_size = 64 * 1024; /* plenty of space for struct + tables */
    void *buf = malloc(buf_size);
    if (!buf) {
        perror("malloc");
        return 1;
    }
    TIFFYCbCrToRGB *ctx = (TIFFYCbCrToRGB *)buf;

    /* Luma coefficients where LumaGreen == 0 triggers division by zero in
       f2 = LumaRed * f1 / LumaGreen (and similarly for f4). */
    float luma[3] = { 0.299f, 0.0f, 0.114f }; /* LumaRed, LumaGreen(=0), LumaBlue */

    /* ReferenceBlackWhite values: valid, but not important since the crash happens earlier. */
    float refBlackWhite[6] = { 0.0f, 255.0f, 0.0f, 255.0f, 0.0f, 255.0f };

    /* This call will hit: f2 = LumaRed * f1 / LumaGreen with LumaGreen == 0,
       which raises SIGFPE due to FE_DIVBYZERO being enabled. */
    (void)TIFFYCbCrToRGBInit(ctx, luma, refBlackWhite);

    /* If we somehow returned (e.g., if FP exceptions are not enabled), clean up. */
    free(buf);
    return 0;
}
