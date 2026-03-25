#define _GNU_SOURCE 1
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <fenv.h>

/* Use libtiff's internal color conversion API */
#include "libtiff/tif_color.h"

/* Prototypes (normally provided by tif_color.h, but redeclare to be explicit) */
extern int TIFFCIELabToRGBInit(TIFFCIELabToRGB *cielab, const TIFFDisplay *display, float *refWhite);
extern void TIFFXYZToRGB(TIFFCIELabToRGB *cielab, float X, float Y, float Z, uint32_t *r, uint32_t *g, uint32_t *b);

int main(void)
{
    /* Enable floating-point divide-by-zero exceptions so the bug is observable as SIGFPE */
#ifdef FE_DIVBYZERO
    feenableexcept(FE_DIVBYZERO);
#endif

    TIFFDisplay display;
    memset(&display, 0, sizeof(display));

    /* Identity matrix: pass XYZ through */
    display.d_mat[0][0] = 1.0f; display.d_mat[0][1] = 0.0f; display.d_mat[0][2] = 0.0f;
    display.d_mat[1][0] = 0.0f; display.d_mat[1][1] = 1.0f; display.d_mat[1][2] = 0.0f;
    display.d_mat[2][0] = 0.0f; display.d_mat[2][1] = 0.0f; display.d_mat[2][2] = 1.0f;

    /* Set ranges so that:
     *  - Red has a non-zero step (to get past the first division)
     *  - Green has zero step: d_YCG == d_Y0G -> gstep = (d_YCG - d_Y0G)/range = 0
     *  - Blue has a non-zero step (not strictly needed)
     */
    display.d_Y0R = 0.0f; display.d_YCR = 1.0f;  /* rstep > 0 */
    display.d_Y0G = 0.0f; display.d_YCG = 0.0f;  /* gstep == 0 triggers divide-by-zero */
    display.d_Y0B = 0.0f; display.d_YCB = 1.0f;  /* bstep > 0 */

    /* Output value ranges (clip limits) */
    display.d_Vrwr = 255u; display.d_Vrwg = 255u; display.d_Vrwb = 255u;

    /* Gammas must be non-zero to avoid earlier division-by-zero in init */
    display.d_gammaR = 1.0f;
    display.d_gammaG = 1.0f;
    display.d_gammaB = 1.0f;

    TIFFCIELabToRGB state;
    float refWhite[3] = { 1.0f, 1.0f, 1.0f };

    if (TIFFCIELabToRGBInit(&state, &display, refWhite) != 0) {
        fprintf(stderr, "Init OK. Triggering TIFFXYZToRGB...\n");
    } else {
        fprintf(stderr, "Init failed (unexpected)\n");
        return 1;
    }

    uint32_t r = 0, g = 0, b = 0;

    /* Any inputs are fine; division by zero occurs when computing green index */
    float X = 0.5f, Y = 0.5f, Z = 0.5f;

    /* This call will perform: i = (Yg - d_Y0G) / gstep; with gstep == 0 -> FP divide-by-zero */
    TIFFXYZToRGB(&state, X, Y, Z, &r, &g, &b);

    /* If we reached here (e.g., FP exceptions masked), print results */
    printf("RGB: %u %u %u\n", r, g, b);
    return 0;
}
