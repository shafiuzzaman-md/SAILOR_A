#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>

/* Types and globals as used by contrib/gregbook/readppm.c */
typedef unsigned char uch;
typedef unsigned long ulg;

#define PROGNAME "reproducer"
#define Trace(x) ((void)0)

static ulg width = 0;       /* crafted to be 0 to make rowbytes == 0 */
static ulg height = 5;      /* any positive value */
static int channels = 3;    /* typical RGB */
static FILE *saved_infile = NULL;
static uch *image_data = NULL;

/* Vulnerable function replicated from contrib/gregbook/readppm.c */
uch *readpng_get_image(double display_exponent, int *pChannels, ulg *pRowbytes)
{
    (void)display_exponent; /* unused in this simplified context */
    ulg rowbytes;

    /* rowbytes becomes 0 when width == 0 */
    *pRowbytes = rowbytes = channels * width;
    *pChannels = channels;

    Trace((stderr, "readpng_get_image:  rowbytes = %ld, height = %ld\n", rowbytes, height));

    /* Guard against integer overflow (vulnerable to divide-by-zero when rowbytes == 0) */
    if (height > ((size_t)(-1)) / rowbytes) {
        fprintf(stderr, PROGNAME ":  image_data buffer would be too large\n");
        return NULL;
    }

    if ((image_data = (uch *)malloc(rowbytes * height)) == NULL) {
        return NULL;
    }

    if (fread(image_data, 1L, rowbytes * height, saved_infile) < rowbytes * height) {
        free(image_data);
        image_data = NULL;
        return NULL;
    }

    return image_data;
}

int main(void)
{
    /* Set crafted dimensions so that rowbytes == channels * width == 0 */
    width = 0;      /* triggers rowbytes = 0 */
    height = 8;     /* non-zero so the guard condition is evaluated */
    channels = 3;   /* RGB */

    int out_channels = 0;
    ulg out_rowbytes = 0;

    /* This call will hit the divide-by-zero in the overflow guard */
    uch *img = readpng_get_image(1.0, &out_channels, &out_rowbytes);

    /* We should never reach here due to SIGFPE from divide-by-zero */
    if (img) free(img);
    puts("Unexpectedly survived divide-by-zero");
    return 0;
}
