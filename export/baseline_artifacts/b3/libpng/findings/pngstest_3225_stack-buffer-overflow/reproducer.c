#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* This reproducer extracts the vulnerable pattern from
 * contrib/libtests/pngstest.c: write_one_file at line 3225.
 * It demonstrates the stack-buffer-overflow caused by:
 *   sprintf(name, "%s%u.png", tmpf, ++counter);
 * where 'name' is a fixed-size 32-byte stack buffer and 'tmpf' is a long prefix.
 */

#define USE_FILE 0x1

typedef struct TestImage {
    unsigned int opts;
} TestImage;

__attribute__((noinline))
static void write_one_file(TestImage *image, const char *tmpf)
{
    if (image->opts & USE_FILE)
    {
        static unsigned int counter = 0;
        char name[32]; /* Vulnerable fixed-size buffer */

        /* This is the vulnerable call from pngstest.c (line 3225). */
        /* Passing a long tmpf will overflow 'name'. */
        sprintf(name, "%s%u.png", tmpf, ++counter);

        /* This line may or may not be reached depending on ASan catching the overflow. */
        (void)fprintf(stderr, "Generated name: %s\n", name);
    }
}

int main(void)
{
    /* Set opts so the USE_FILE path is taken. */
    TestImage img;
    img.opts = USE_FILE;

    /* Craft a long prefix to overflow the 32-byte 'name' buffer.
     * Length calculation:
     *   prefix_len + digits(counter) + strlen(".png") must exceed 31.
     * Use a much longer prefix to ensure overflow reliably.
     */
    const size_t prefix_len = 80; /* comfortably larger than 31 */
    char *long_prefix = (char *)malloc(prefix_len + 1);
    if (!long_prefix) return 1;
    memset(long_prefix, 'A', prefix_len);
    long_prefix[prefix_len] = '\0';

    write_one_file(&img, long_prefix);

    free(long_prefix);
    return 0;
}
