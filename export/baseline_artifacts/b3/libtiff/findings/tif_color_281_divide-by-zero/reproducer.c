#define _GNU_SOURCE 1
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fenv.h>
#include <libtiff/tiffio.h>

/* glibc provides feenableexcept as a GNU extension. Declare weakly so build
 * still succeeds if it's unavailable. */
int feenableexcept(int) __attribute__((weak));

int main(void) {
    /* Enable trapping of floating-point divide-by-zero to make the bug observable */
    #pragma STDC FENV_ACCESS ON
    feclearexcept(FE_ALL_EXCEPT);
    if (feenableexcept) {
        feenableexcept(FE_DIVBYZERO | FE_INVALID | FE_OVERFLOW);
    }

    /* Allocate a large buffer so TIFFYCbCrToRGBInit can place its tables
     * after the struct as it expects. */
    size_t bufsize = 65536; /* plenty of space */
    TIFFYCbCrToRGB *ycbcr = (TIFFYCbCrToRGB *)malloc(bufsize);
    if (!ycbcr) {
        perror("malloc");
        return 1;
    }

    /* Luma coefficients with LumaGreen = 0.0f triggers division by zero in:
     *   f2 = LumaRed * f1 / LumaGreen
     *   f4 = LumaBlue * f3 / LumaGreen
     */
    float luma[3] = { 0.299f, 0.0f, 0.114f }; /* Red, Green(=0), Blue */

    /* Reasonable RefBlackWhite values (Yb, Yw, Cbb, Cbw, Crb, Crw). */
    float refBlackWhite[6] = { 0.0f, 255.0f, 0.0f, 255.0f, 0.0f, 255.0f };

    /* Call into the vulnerable function. With FE_DIVBYZERO trapping enabled,
     * this should raise SIGFPE due to float division by zero. */
    int ret = TIFFYCbCrToRGBInit(ycbcr, luma, refBlackWhite);
    printf("TIFFYCbCrToRGBInit returned %d (no trap?)\n", ret);

    free(ycbcr);
    return 0;
}
