#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/*
 * Minimal, self-contained replica of the vulnerable logic in
 * htmlParseCharDataInternal from HTMLparser.c. The bug is that in the '\r'
 * case, it reads in[1] without ensuring that at least two bytes are
 * available when partial == 0 and avail == 1.
 */
static void htmlParseCharDataInternal(const unsigned char *in, int avail, int partial) {
    int skip = 0;
    const unsigned char *repl = (const unsigned char *)"";
    int replSize = 0;
    int done = 0;
    int line = 1;
    int col = 1;

    if (avail <= 0 || in == NULL) return;

    unsigned char cur = in[0];

    switch (cur) {
        case '\0':
            skip = 1;
            repl = (const unsigned char *)"";
            replSize = 0;
            goto next_chunk;

        case '\n':
            line += 1;
            col = 1;
            break;

        case '\r':
            /*
             * Vulnerable condition: if partial is false, the guard below is
             * bypassed even when avail == 1, so the code reads in[1].
             */
            if (partial && avail < 2) {
                done = 1;
                goto next_chunk;
            }

            skip = 1;
            /*
             * Out-of-bounds read when avail == 1 and partial == 0.
             * Accesses in[1] which is past the end of the buffer.
             */
            if (in[1] != 0x0A) {
                repl = (const unsigned char *)"\x0A";
                replSize = 1;
            }
            goto next_chunk;

        default:
            /* Not relevant for this reproducer */
            break;
    }

next_chunk:
    /* Use variables to avoid being optimized out (though we compile with -O0). */
    (void)skip;
    (void)repl;
    (void)replSize;
    (void)done;
    (void)line;
    (void)col;
}

int main(void) {
    /* Prepare a 1-byte buffer containing a single '\r' to simulate EOF with avail == 1. */
    unsigned char *buf = (unsigned char *)malloc(1);
    if (!buf) {
        perror("malloc");
        return 1;
    }
    buf[0] = '\r';

    /*
     * Trigger conditions:
     * - avail == 1 (only one byte available)
     * - partial == 0 (so the guard is bypassed)
     * This will cause htmlParseCharDataInternal to read buf[1], which is OOB.
     */
    htmlParseCharDataInternal(buf, 1, 0);

    /* If ASan didn't already abort, clean up. */
    free(buf);
    return 0;
}
