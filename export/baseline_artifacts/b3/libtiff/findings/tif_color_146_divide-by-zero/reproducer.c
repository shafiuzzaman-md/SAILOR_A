#include <stdio.h>
#include <string.h>
#include <fenv.h>
#include <tiffio.h>

/* feenableexcept is a GNU extension; declare it explicitly to avoid missing prototype warnings */
extern int feenableexcept(int);

int main(void) {
    /* Enable floating-point exceptions so 1.0/0.0 triggers SIGFPE */
    feenableexcept(FE_DIVBYZERO | FE_INVALID | FE_OVERFLOW);

    TIFFCIELabToRGB cielab;            /* Conversion state (created on stack) */
    TIFFDisplay disp;                  /* Display parameters provided to init */
    float refWhite[3] = { 1.0f, 1.0f, 1.0f }; /* Any valid ref white */

    /* Initialize display parameters with sane defaults, except set d_gammaR=0 to trigger divide-by-zero */
    memset(&disp, 0, sizeof(disp));

    /* Identity matrix (not used by the immediate crashing path, but set for completeness) */
    disp.d_mat[0][0] = 1.0f; disp.d_mat[0][1] = 0.0f; disp.d_mat[0][2] = 0.0f;
    disp.d_mat[1][0] = 0.0f; disp.d_mat[1][1] = 1.0f; disp.d_mat[1][2] = 0.0f;
    disp.d_mat[2][0] = 0.0f; disp.d_mat[2][1] = 0.0f; disp.d_mat[2][2] = 1.0f;

    /* Luminance and offsets */
    disp.d_YCR = 1.0f; disp.d_YCG = 1.0f; disp.d_YCB = 1.0f;
    disp.d_Y0R = 0.0f; disp.d_Y0G = 0.0f; disp.d_Y0B = 0.0f;

    /* White point voltages (arbitrary, but reasonable) */
    disp.d_Vrwr = 255.0f; disp.d_Vrwg = 255.0f; disp.d_Vrwb = 255.0f;

    /* Set gamma values: RED gamma is zero to trigger the bug; others valid */
    disp.d_gammaR = 0.0f;   /* This will cause dfGamma = 1.0 / 0.0 */
    disp.d_gammaG = 2.2f;
    disp.d_gammaB = 2.2f;

    /* Call into the vulnerable initializer. The divide-by-zero occurs at line ~146 in tif_color.c */
    /* This should raise SIGFPE due to enabled FP exceptions. */
    (void)TIFFCIELabToRGBInit(&cielab, &disp, refWhite);

    /* If we get here, the divide-by-zero didn't trap (unexpected with feenableexcept). */
    puts("TIFFCIELabToRGBInit returned without FP exception (unexpected)");
    return 0;
}
