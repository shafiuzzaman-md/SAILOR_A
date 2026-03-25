#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* This reproducer embeds the vulnerable Windows-specific logic directly. */

static char prog[40];

/* Vulnerable helper: returns pointer one byte before buffer when filename is empty */
const char *opt_path_end(const char *filename)
{
    const char *p;

    /* find the last '/', '\\' or ':' */
    for (p = filename + strlen(filename); --p > filename;)
        if (*p == '/' || *p == '\\' || *p == ':') {
            p++;
            break;
        }
    return p;
}

/* Vulnerable function: calls strlen() on pointer returned from opt_path_end */
char *opt_progname(const char *argv0)
{
    size_t i, n;
    const char *p;
    char *q;

    p = opt_path_end(argv0);

    /* Strip off trailing nonsense. */
    n = strlen(p); /* OOB-read when argv0 is empty: p == argv0 - 1 */
    if (n > 4 && (strcmp(&p[n - 4], ".exe") == 0 || strcmp(&p[n - 4], ".EXE") == 0))
        n -= 4;

    /* Copy over the name, in lowercase. */
    if (n > sizeof(prog) - 1)
        n = sizeof(prog) - 1;
    for (q = prog, i = 0; i < n; i++, p++)
        *q++ = (char)tolower((unsigned char)*p);
    *q = '\0';
    return prog;
}

int main(void)
{
    /* Allocate a 1-byte heap buffer holding an empty string. */
    char *empty = (char *)malloc(1);
    if (!empty) {
        perror("malloc");
        return 1;
    }
    empty[0] = '\0';

    /* This triggers the bug: opt_path_end returns empty - 1, strlen() reads OOB. */
    char *name = opt_progname(empty);

    /* If ASan doesn't abort before, print to avoid optimizing away. */
    printf("progname: %s\n", name);

    free(empty);
    return 0;
}
