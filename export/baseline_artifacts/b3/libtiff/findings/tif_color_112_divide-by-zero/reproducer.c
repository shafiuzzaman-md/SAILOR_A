#define _GNU_SOURCE
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <fenv.h>
#include <string.h>
#include <stdlib.h>
#include "tiffio.h"

/* The function is defined in libtiff but not necessarily declared in public headers. */
extern void TIFFXYZToRGB(TIFFCIELabToRGB *cielab, float X, float Y, float Z,
                         uint32_t *r, uint32_t *g, uint32_t *b);

/* Also declare the init function (available in public API). */
extern int TIFFCIELabToRGBInit(TIFFCIELabToRGB *cielab, const TIFFDisplay *display,
                               float *refWhite);

static void enable_fp_traps(void) {
    /* Trap floating point divide-by-zero and invalid operations so the bug
       manifests as a crash (SIGFPE) instead of producing Inf/NaN. */
#if defined(__linux__)
    feenableexcept(FE_DIVBYZERO | FE_INVALID);
#endif
}

int main(void) {
    enable_fp_traps();

    TIFFCIELabToRGB cielab;
    memset(&cielab, 0, sizeof(cielab));

    TIFFDisplay display;
    memset(&display, 0, sizeof(display));

    /* Set a simple XYZ->RGB matrix. We'll make Yr depend on X to keep things simple. */
    /* Yr = 1*X + 0*Y + 0*Z; Yg = 0*X + 1*Y + 0*Z; Yb = 0*X + 0*Y + 1*Z */
    display.d_mat[0][0] = 1.0f; display.d_mat[0][1] = 0.0f; display.d_mat[0][2] = 0.0f;
    display.d_mat[1][0] = 0.0f; display.d_mat[1][1] = 1.0f; display.d_mat[1][2] = 0.0f;
    display.d_mat[2][0] = 0.0f; display.d_mat[2][1] = 0.0f; display.d_mat[2][2] = 1.0f;

    /* Valid luminance ranges so clipping won't force numerator to 0. */
    display.d_Y0R = 0.0f; display.d_YCR = 1.0f;
    display.d_Y0G = 0.0f; display.d_YCG = 1.0f;
    display.d_Y0B = 0.0f; display.d_YCB = 1.0f;

    /* Output clip limits (8-bit range is fine). */
    display.d_Vrwr = 255; display.d_Vrwg = 255; display.d_Vrwb = 255;

    /* Reasonable gamma values. */
    display.d_gammaR = 2.2f; display.d_gammaG = 2.2f; display.d_gammaB = 2.2f;

    /* D65 reference white. */
    float refWhite[3] = { 0.9505f, 1.0f, 1.0890f };

    if (TIFFCIELabToRGBInit(&cielab, &display, refWhite) != 0) {
        /* Force the bug: make rstep exactly zero while keeping a valid display range. */
        cielab.rstep = 0.0f;
        /* Keep gstep/bstep non-zero to ensure we hit the first divide in R path. */
        if (cielab.gstep == 0.0f) cielab.gstep = 1.0f;
        if (cielab.bstep == 0.0f) cielab.bstep = 1.0f;

        /* Choose X so that (Yr - d_Y0R) is non-zero after clipping.
           With the matrix above, Yr = X, and d_Y0R = 0.0, so any X>0 works. */
        float X = 0.6f, Y = 0.4f, Z = 0.2f;
        uint32_t r = 0, g = 0, b = 0;

        /* This call will compute: i = (Yr - d_Y0R) / rstep; with rstep == 0.0f
           and Yr - d_Y0R > 0, triggering a floating-point divide-by-zero. */
        TIFFXYZToRGB(&cielab, X, Y, Z, &r, &g, &b);

        /* If FP exceptions are not trapped on this platform, print results to
           prevent the call from being optimized away and to observe behavior. */
        printf("r=%u g=%u b=%u (this line may not be reached if SIGFPE is raised)\n", r, g, b);
    } else {
        fprintf(stderr, "TIFFCIELabToRGBInit failed\n");
    }

    return 0;
}
