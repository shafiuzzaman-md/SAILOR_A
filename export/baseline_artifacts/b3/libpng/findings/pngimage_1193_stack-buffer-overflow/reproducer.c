#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>

/* Minimal typedefs/constants to mirror pngimage.c environment */
typedef unsigned char png_byte;

#define LIBPNG_BUG 1
#define INTERNAL_ERROR 2

/* Stubbed logger used by the vulnerable code */
static void display_log(void *dp, int code, const char *fmt, ...)
{
    (void)dp; (void)code;
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fputc('\n', stderr);
    va_end(ap);
}

/* Reimplementation of the vulnerable function's sBIT handling path.
 * This mirrors the logic around contrib/libtests/pngimage.c:1184-1196.
 */
static void compare_read(void)
{
    /* Stack array that will be overflowed */
    png_byte sig_bits[8];

    /* Craft conditions: 16-bit depth and 4 components (RGBA) => bpp = 4 * 16 = 64 */
    int bit_depth = 16;
    unsigned int bpp = 64; /* bits-per-pixel */

    /* Initialize sig_bits to non-zero values within range to pass earlier checks */
    for (int i = 0; i < 8; ++i)
        sig_bits[i] = 1;

    /* Preceding validation loop from pngimage.c (safe, stays in-bounds here) */
    for (int b = 0; 8U * (unsigned)b < bpp; ++b)
    {
        if (sig_bits[b] == 0 || sig_bits[b] > bit_depth /*!palette*/)
            display_log(NULL, LIBPNG_BUG,
                        "invalid sBIT[%u]  value %d returned for PNG bit depth %d",
                        b, sig_bits[b], bit_depth);
    }

    /* Vulnerable 16-bit path: for RGBA16, (bpp >> 4) == 4.
     * The loop writes to sig_bits[2*b+1] and sig_bits[2*b].
     * When b == 4, writes hit indices 9 and 8 (OOB for 8-byte array).
     */
    switch (bit_depth)
    {
        int b;
        case 16: /* Two bytes per component, big-endian */
            for (b = (bpp >> 4); b > 0; --b)
            {
                unsigned int sig = (unsigned int)(0xffff0000U >> sig_bits[b]);
                /* Out-of-bounds writes when b == 4: indices 9 and 8 */
                sig_bits[2*b+1] = (png_byte)sig;
                sig_bits[2*b+0] = (png_byte)(sig >> 8); /* big-endian */
            }
            break;
        default:
            break;
    }

    /* Prevent the compiler from optimizing the array away */
    volatile unsigned int sum = 0;
    for (int i = 0; i < 8; ++i) sum += sig_bits[i];
    fprintf(stderr, "sum=%u\n", sum);
}

int main(void)
{
    compare_read();
    return 0;
}
