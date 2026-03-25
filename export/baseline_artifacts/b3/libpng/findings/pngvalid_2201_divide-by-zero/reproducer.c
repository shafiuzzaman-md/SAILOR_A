#define _GNU_SOURCE
#pragma STDC FENV_ACCESS ON
#include <stdio.h>
#include <fenv.h>
#include <stdlib.h>

/* GNU extension to enable trapping FP exceptions. Declare explicitly in case
 * the prototype is not exposed by default headers on the system. */
extern int feenableexcept(int);

/* Minimal re-declarations from contrib/libtests/pngvalid.c */
typedef struct CIE_color {
    double X, Y, Z;
} CIE_color;

/* Vulnerable function from pngvalid.c (no denominator check) */
static double chromaticity_x(CIE_color c)
{
    return c.X / (c.X + c.Y + c.Z);
}

int main(void)
{
    /* Make floating-point divide-by-zero (and invalid) raise SIGFPE so the
     * bug is visible when running under ASan-only builds. */
    feclearexcept(FE_ALL_EXCEPT);
    feenableexcept(FE_DIVBYZERO | FE_INVALID);

    /* Craft inputs so the denominator (X+Y+Z) is exactly zero while the
     * numerator (X) is non-zero, which triggers a true floating-point
     * divide-by-zero exception (FE_DIVBYZERO).
     *
     * Note: Using X=Y=Z=0 would yield 0.0/0.0, which triggers FE_INVALID, so
     * enabling FE_INVALID above would also reproduce the issue. */
    CIE_color c;
    c.X = 1.0;
    c.Y = -0.5;
    c.Z = -0.5;  /* X + Y + Z == 0.0 */

    volatile double r = chromaticity_x(c); /* Triggers SIGFPE here */
    /* The line below should never execute due to the SIGFPE. */
    printf("Result: %f\n", r);

    return 0;
}
