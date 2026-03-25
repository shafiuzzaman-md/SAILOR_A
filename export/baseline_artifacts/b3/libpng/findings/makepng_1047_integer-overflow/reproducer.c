#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>

/* Minimal type used by libpng */
typedef unsigned char png_byte;

/*
 * This function mimics the vulnerable allocation site in
 * contrib/libtests/makepng.c:load_file() around line 1047.
 *
 * In the real code, 'total' is a size_t counting the bytes in an input file,
 * then the allocation size is computed as (total+3)&~3. On 32-bit size_t, if
 * total is near SIZE_MAX, total+3 wraps to a small value (possibly 0), but the
 * subsequent copy loop writes 'total' bytes, overflowing the heap buffer.
 *
 * To reproduce this on 64-bit hosts without needing a multi-gigabyte input,
 * we explicitly perform the allocation rounding in 32-bit arithmetic to
 * simulate the 32-bit overflow, then we mimic the copy loop by writing a small
 * number of bytes into the under-allocated buffer. AddressSanitizer will catch
 * the resulting heap-buffer-overflow.
 */
static size_t load_file(const char *name, png_byte **result)
{
    (void)name; /* unused in this self-contained reproducer */

    /* Craft a 'total' that would overflow 32-bit when adding 3. */
    size_t total = (size_t)UINT32_MAX - 1; /* 0xFFFFFFFE */

    /* Vulnerable allocation rounding done in 32-bit to simulate 32-bit size_t */
    uint32_t alloc32 = (uint32_t)(total + 3);      /* wraps to 1 */
    alloc32 &= ~3U;                                /* rounds down to 0 */

    /* This mirrors: png_byte *data = malloc((total+3)&~3); */
    png_byte *data = (png_byte*)malloc((size_t)alloc32); /* likely size 0 */

    /* In the real code there is an 'if (data != NULL)' check, but malloc(0)
     * may legally return NULL or non-NULL. To deterministically demonstrate
     * the overflow even if malloc(0) returns NULL on the platform, fall back
     * to a tiny allocation that is still far smaller than 'total'.
     */
    if (data == NULL) {
        data = (png_byte*)malloc(8); /* tiny under-allocation */
    }

    /* Mimic the read-back loop at line 1057:
     *   data[new_size++] = (png_byte)ch;
     * We only need to write a few bytes to overflow the tiny buffer.
     */
    size_t new_size = 0;
    for (size_t i = 0; i < 64; ++i) {
        data[new_size++] = (png_byte)'A'; /* heap-buffer-overflow here */
    }

    *result = data;
    return total;
}

int main(void)
{
    png_byte *buf = NULL;
    size_t total = load_file("dummy", &buf);

    /* Prevent optimizing away and keep program flow simple. */
    if (buf != NULL) {
        fprintf(stderr, "Simulated total=%zu, first byte=%u\n", total, (unsigned)buf[0]);
        free(buf);
    }

    return 0;
}
