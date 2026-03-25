#define _GNU_SOURCE
#include <stdio.h>
#include <fenv.h>
#include <math.h>

/* Replicated minimal types and functions from the vulnerable code */
typedef struct CIE_color {
    double X, Y, Z;
} CIE_color;

typedef struct color_encoding {
    double gamma; /* unused here */
    CIE_color red, green, blue;
} color_encoding;

static double chromaticity_y(CIE_color c)
{
    /* Vulnerable: no check for (X + Y + Z) == 0 */
    return c.Y / (c.X + c.Y + c.Z);
}

static CIE_color white_point(const color_encoding *encoding)
{
    CIE_color white;
    white.X = encoding->red.X + encoding->green.X + encoding->blue.X;
    white.Y = encoding->red.Y + encoding->green.Y + encoding->blue.Y;
    white.Z = encoding->red.Z + encoding->green.Z + encoding->blue.Z;
    return white;
}

int main(void)
{
    /* Enable floating-point exceptions so the 0/0 triggers SIGFPE (FE_INVALID) */
#if defined(__gnu_linux__) || defined(__GLIBC__)
    /* feenableexcept is a GNU extension; declare prototype if missing */
    extern int feenableexcept(int);
    feenableexcept(FE_INVALID | FE_DIVBYZERO);
#endif

    /* Use volatile to prevent compile-time constant folding of zeros */
    volatile double v0 = 0.0;

    color_encoding enc;
    enc.gamma = 1.0;
    enc.red.X = v0;   enc.red.Y = v0;   enc.red.Z = v0;
    enc.green.X = v0; enc.green.Y = v0; enc.green.Z = v0;
    enc.blue.X = v0;  enc.blue.Y = v0;  enc.blue.Z = v0;

    /* This produces a white point with X=Y=Z=0 */
    CIE_color white = white_point(&enc);

    /* This call divides 0.0 by (0.0+0.0+0.0) => 0.0/0.0 -> FE_INVALID */
    double y = chromaticity_y(white);

    /* If exceptions are masked on this platform, print the NaN result */
    printf("chromaticity_y = %f\n", y);
    return 0;
}
