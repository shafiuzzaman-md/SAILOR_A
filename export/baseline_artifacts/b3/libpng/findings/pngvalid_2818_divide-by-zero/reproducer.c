#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Minimal re-declaration of the structure and functions needed
 * to trigger the bug described from contrib/libtests/pngvalid.c
 */

typedef struct png_modifier {
    int repeat;
    int test_uses_encoding;
    int test_exhaustive;
    unsigned int encoding_counter;
    unsigned int ngammas;
    unsigned int nencodings;
    int assume_16_bit_calculations;
    unsigned int bit_depth;
} png_modifier;

/* Mirrors the logic in modifier_total_encodings from pngvalid.c */
static unsigned int modifier_total_encodings(const png_modifier *pm)
{
    return 1 /* nothing */
        + pm->ngammas /* gamma values to test */
        + pm->nencodings /* total number of encodings */
        + ((pm->bit_depth == 16 || pm->assume_16_bit_calculations) ?
            pm->nencodings : 0); /* encodings with gamma == 1.0 if 16-bit */
}

/* Simple random_mod implementation that divides by the provided modulus. */
static unsigned int random_mod(unsigned int max)
{
    /* This will trigger a divide-by-zero when max == 0 */
    unsigned int r = (unsigned int)rand();
    return r % max; /* divide-by-zero when max == 0 */
}

/* Directly adapted from the vulnerable code path. */
static void modifier_encoding_iterate(png_modifier *pm)
{
    if (!pm->repeat && pm->test_uses_encoding)
    {
        if (pm->test_exhaustive)
        {
            if (++pm->encoding_counter >= modifier_total_encodings(pm))
                pm->encoding_counter = 0; /* This will stop the repeat */
        }
        else
        {
            /* Not exhaustive - choose an encoding at random; generate a number in
             * the range 1..(max-1), so the result is always non-zero:
             */
            if (pm->encoding_counter == 0)
                pm->encoding_counter = random_mod(modifier_total_encodings(pm)-1) + 1;
            else
                pm->encoding_counter = 0;
        }

        if (pm->encoding_counter > 0)
            pm->repeat = 1;
    }
    else if (!pm->repeat)
        pm->encoding_counter = 0;
}

int main(void)
{
    /* Seed randomness (not required for the bug to trigger) */
    srand(1);

    /* Set up a png_modifier instance that meets the triggering conditions:
     * - non-exhaustive mode (test_exhaustive == 0)
     * - encoding_counter == 0 to enter the random_mod(...) path
     * - test_uses_encoding == 1 to take the encoding-dependent branch
     * - total encodings should compute to 1 so that (max - 1) == 0
     *   which happens when ngammas == 0, nencodings == 0, and not 16-bit only.
     */
    png_modifier pm;
    pm.repeat = 0;
    pm.test_uses_encoding = 1;
    pm.test_exhaustive = 0;
    pm.encoding_counter = 0;
    pm.ngammas = 0;
    pm.nencodings = 0;
    pm.assume_16_bit_calculations = 0;
    pm.bit_depth = 8; /* not 16-bit, so the extra encodings term is 0 */

    /* This call triggers: random_mod(modifier_total_encodings(pm)-1) where
     * modifier_total_encodings(pm) == 1, thus random_mod(0) -> divide-by-zero.
     */
    modifier_encoding_iterate(&pm);

    /* If we reach here without a crash, something went wrong. */
    printf("Unexpected: divide-by-zero did not occur.\n");
    return 0;
}
