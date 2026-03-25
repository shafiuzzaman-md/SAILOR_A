#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>

/* Minimal typedefs to mirror libpng types */
typedef uint32_t png_uint_32;
typedef int32_t  png_int_32;
typedef png_int_32 png_fixed_point;

typedef struct png_struct_def { int dummy; } png_struct;

/* Stubbed error handler (normally aborts inside libpng) */
static void png_error(const png_struct *png_ptr, const char *msg)
{
    (void)png_ptr;
    fprintf(stderr, "png_error: %s\n", msg);
    abort();
}

/* Vulnerable function: mirrors the problematic semantics in png.c:2628
 * Specifically, when fp == INT_MIN (-2147483648), the expression (-fp)
 * overflows a 32-bit signed int before being cast to png_uint_32, which is UB.
 */
void png_ascii_from_fixed(const png_struct *png_ptr, char *ascii,
                          size_t size, png_fixed_point fp)
{
    /* Require space for 10 decimal digits, a decimal point, a minus sign and a
     * trailing \0, 13 characters:
     */
    if (size <= 12)
        png_error(png_ptr, "ASCII conversion buffer too small");

    png_uint_32 num;

    /* Avoid overflow here on the minimum integer. (BUG: does not actually avoid)
     * This reproduces the bug: when fp == INT_MIN, the unary minus overflows.
     */
    if (fp < 0)
    {
        *ascii++ = 45; /* '-' */
        /* BUG: undefined behavior if fp == INT_MIN */
        num = (png_uint_32)(-fp); /* UB when fp == INT_MIN */
    }
    else
        num = (png_uint_32)fp;

    /* Basic formatting into fixed-point with 5 fractional digits, similar in spirit
     * to libpng, just enough to exercise the code path and use 'num'.
     */
    char digits[32];
    unsigned nd = 0;

    if (num == 0) {
        digits[nd++] = '0';
    } else {
        while (num > 0 && nd < sizeof(digits)) {
            unsigned d = (unsigned)(num % 10);
            digits[nd++] = (char)('0' + d);
            num /= 10;
        }
    }

    /* Ensure at least 6 digits so we can insert a decimal point before last 5 */
    while (nd < 6 && nd < sizeof(digits)) digits[nd++] = '0';

    /* Output integer part (digits beyond 5 fractional), then '.', then 5 fractional */
    if (nd > 5) {
        for (int i = (int)nd - 1; i >= 5; --i) *ascii++ = digits[i];
    } else {
        *ascii++ = '0';
    }
    *ascii++ = '.';
    for (int i = 4; i >= 0; --i) *ascii++ = digits[i];
    *ascii = '\0';
}

int main(void)
{
    char buf[64];
    png_struct *png_ptr = NULL; /* not used in this path */

    /* Trigger the UB: fp == INT_MIN causes (-fp) to overflow signed int. */
    png_fixed_point fp = INT_MIN; /* -2147483648 on 32-bit two's complement */

    /* Use sufficiently large buffer to avoid png_error path */
    png_ascii_from_fixed(png_ptr, buf, sizeof(buf), fp);

    /* Print the produced ASCII to keep computations live and observable */
    printf("Formatted: %s\n", buf);

    /* Also run with a safe negative value for comparison */
    png_ascii_from_fixed(png_ptr, buf, sizeof(buf), -123456789);
    printf("Formatted (control): %s\n", buf);

    return 0;
}
