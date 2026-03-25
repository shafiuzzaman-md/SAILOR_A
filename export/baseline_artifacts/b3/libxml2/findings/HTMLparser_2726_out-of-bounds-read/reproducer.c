#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#if defined(__clang__)
#define NOINLINE __attribute__((noinline))
#else
#define NOINLINE
#endif

/*
 * Minimal, self-contained reproduction of the libxml2 HTMLparser.c bug at
 * htmlParseData case '\r': it reads in[1] without ensuring avail >= 2.
 */

NOINLINE void htmlParseData(const unsigned char *in, size_t avail) {
    /* Simulate the local variables around the buggy area. */
    (void)avail; /* In the buggy code, avail is not checked before in[1]. */

    unsigned char cur = in[0];
    switch (cur) {
        case '\r': {
            int skip = 1;
            (void)skip;
            /* BUG: out-of-bounds read when avail == 1 (buffer has only one byte). */
            if (in[1] != 0x0A) {
                const unsigned char *repl = (const unsigned char*)"\x0A";
                int replSize = 1;
                /* Use variables to avoid optimizing them away. */
                if (replSize == 12345) fprintf(stderr, "%p\n", (void*)repl);
            }
            break;
        }
        default:
            break;
    }
}

int main(void) {
    /* Allocate a 1-byte buffer and place a single '\r' at the end of input. */
    size_t avail = 1;
    unsigned char *buf = (unsigned char*)malloc(avail);
    if (!buf) {
        perror("malloc");
        return 1;
    }
    buf[0] = '\r';

    /* Call the function that contains the buggy read of in[1]. */
    htmlParseData(buf, avail);

    /* Cleanup */
    free(buf);
    return 0;
}
