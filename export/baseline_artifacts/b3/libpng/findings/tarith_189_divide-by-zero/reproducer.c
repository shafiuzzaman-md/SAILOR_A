#define _GNU_SOURCE
#include <assert.h>
#include <fenv.h>
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Minimal stand-ins for the libpng floating-point state helpers */
#define PNG_FP_IS_NEGATIVE(s) ((s) & 0x01)
#define PNG_FP_IS_ZERO(s)     ((s) & 0x02)
#define PNG_FP_IS_POSITIVE(s) ((s) & 0x04)

/* Stubbed validator: claim the string is a valid FP number, set index to end,
 * and report the value as ZERO (only to satisfy the sign/zero checks). */
static int png_check_fp_number(const char *str, int max/*unused*/, int *state, int *index)
{
    (void)max;
    if (index) *index = (int)strlen(str);
    if (state) *state = 0x02; /* ZERO bit set, not negative, not positive */
    return 1; /* valid */
}

/* This is a minimal reproduction of the vulnerable code path inside
 * contrib/libtests/tarith.c:validation_ascii_to_fp around line 189. */
static void validation_ascii_to_fp_reproducer(void)
{
    int precision = 1;
    int failed = 0;
    int showall = 0;
    double max_error_abs = 0.0, max_error = 0.0;
    double max_abs = 0.0, max = 0.0;
    int minorarith = 0;

    /* The original bug arises because 'test' is initialized to 0 before the
     * first loop iteration. We mimic just that first iteration. */
    double test = 0.0; /* initialized to 0 => division by zero below */

    /* The formatted buffer for 'test'. To deterministically trigger a floating
     * point divide-by-zero (FE_DIVBYZERO rather than FE_INVALID 0/0), we choose
     * a non-zero string so (out - test) != 0 while test == 0. */
    char buffer[8] = "1"; /* out becomes 1.0 */

    int state = 0, index = 0;

    if (!png_check_fp_number(buffer, precision+10, &state, &index) || buffer[index] != 0)
    {
        fprintf(stderr, "%g[%d] -> '%s' but has bad format ('%c')\n",
                test, precision, buffer, buffer[index]);
        failed = 1;
    }
    else if (PNG_FP_IS_NEGATIVE(state) && !(test < 0))
    {
        fprintf(stderr, "%g[%d] -> '%s' but negative value not so reported\n",
                test, precision, buffer);
        failed = 1;
        assert(!PNG_FP_IS_ZERO(state));
        assert(!PNG_FP_IS_POSITIVE(state));
    }
    else if (PNG_FP_IS_ZERO(state) && !(test == 0))
    {
        fprintf(stderr, "%g[%d] -> '%s' but zero value not so reported\n",
                test, precision, buffer);
        failed = 1;
        assert(!PNG_FP_IS_NEGATIVE(state));
        assert(!PNG_FP_IS_POSITIVE(state));
    }
    else if (PNG_FP_IS_POSITIVE(state) && !(test > 0))
    {
        fprintf(stderr, "%g[%d] -> '%s' but positive value not so reported\n",
                test, precision, buffer);
        failed = 1;
        assert(!PNG_FP_IS_NEGATIVE(state));
        assert(!PNG_FP_IS_ZERO(state));
    }
    else
    {
        /* This is the vulnerable block. In the first iteration test==0.
         * With buffer="1", out==1, so (out - test)/test is 1/0 -> FP div-by-zero. */
        double out = atof(buffer);
        double change = fabs((out - test)/test); /* divide-by-zero here */
        double allow = .5 / pow(10, (precision >= DBL_DIG) ? DBL_DIG-1 : precision-1);

        if (change >= allow && (isfinite(out) || fabs(test/DBL_MAX) <= 1-allow))
        {
            double percent = (precision >= DBL_DIG) ? max_error_abs : max_error;
            double allowp = (change-allow)*100/allow;

            if (precision >= DBL_DIG)
            {
                if (max_abs < allowp) max_abs = allowp;
            }
            else
            {
                if (max < allowp) max = allowp;
            }

            if (showall || allowp >= percent)
            {
                fprintf(stderr,
                        "%.*g[%d] -> '%s' -> %.*g number changed (%g > %g (%d%%))\n",
                        DBL_DIG, test, precision, buffer, DBL_DIG, out, change, allow,
                        (int)round(allowp));
                failed = 1;
            }
            else
                ++minorarith;
        }
    }

    (void)failed; (void)minorarith; (void)max_abs; (void)max; /* silence unused if any */
}

int main(void)
{
    /* Enable trapping of FP exceptions so the divide-by-zero is observable as a crash. */
#if defined(__GLIBC__) || defined(__GNU_LIBRARY__)
    feenableexcept(FE_DIVBYZERO | FE_INVALID);
#endif
    validation_ascii_to_fp_reproducer();
    return 0;
}
