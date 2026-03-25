#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <fenv.h>

/* Ensure we take the floating-point path in png_reciprocal */
#define PNG_FLOATING_ARITHMETIC_SUPPORTED 1

typedef long png_fixed_point;

/* Vulnerable implementation extracted/simplified from png.c */
png_fixed_point png_reciprocal(png_fixed_point a)
{
#ifdef PNG_FLOATING_ARITHMETIC_SUPPORTED
    /* BUG: no check for a == 0, performs 1E10 / a */
    double r = floor(1E10 / a + .5);

    if (r <= 2147483647. && r >= -2147483648.)
        return (png_fixed_point)r;
#else
    png_fixed_point res;
    if (png_muldiv(&res, 100000, 100000, a) != 0)
        return res;
#endif

    return 0; /* error/overflow */
}

int main(void)
{
    /* Clear and enable floating-point exceptions so div-by-zero traps.
     * feenableexcept is a GNU extension provided by glibc/libm.
     */
    feclearexcept(FE_ALL_EXCEPT);
#ifdef FE_DIVBYZERO
    /* Enable trap on floating-point divide-by-zero (and related) */
    feenableexcept(FE_DIVBYZERO | FE_INVALID | FE_OVERFLOW);
#endif

    volatile png_fixed_point a = 0; /* Craft input that triggers the bug */

    /* This call will perform 1E10 / 0.0 in the floating path above, raising
     * a floating-point exception (SIGFPE) on platforms where traps are enabled.
     */
    png_fixed_point res = png_reciprocal(a);

    /* If traps are not enabled, execution may continue and res is likely 0.
     * We print the result so the compiler cannot optimize the call away.
     */
    printf("png_reciprocal(0) -> %ld\n", (long)res);
    return 0;
}
