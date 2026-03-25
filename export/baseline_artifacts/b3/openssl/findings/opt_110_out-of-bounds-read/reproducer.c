#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Self-contained reproduction of the VMS-specific bug in apps/lib/opt.c */

/* Global buffer as used by opt_progname */
static char prog[256];

/* Vulnerable VMS-specific helper: on empty filename, returns filename-1 */
const char *opt_path_end(const char *filename)
{
    const char *p;

    /* Find last special character sys:[foo.bar]openssl */
    for (p = filename + strlen(filename); --p > filename;) {
        if (*p == ':' || *p == ']' || *p == '>') {
            p++;
            break;
        }
    }
    return p; /* With empty filename, p == filename-1 (OOB) */
}

/* Vulnerable function: calls strrchr() on possibly invalid pointer p */
char *opt_progname(const char *argv0)
{
    const char *p, *q;

    /* Find last special character sys:[foo.bar]openssl */
    p = opt_path_end(argv0);
    /* Out-of-bounds read when argv0 is empty string: p == argv0 - 1 */
    q = strrchr(p, '.');

    if (prog != p)
        strncpy(prog, p, sizeof(prog) - 1);
    prog[sizeof(prog) - 1] = '\0';
    if (q != NULL && q - p < (ptrdiff_t)sizeof(prog))
        prog[q - p] = '\0';
    return prog;
}

int main(void)
{
    /* Allocate an empty string on the heap so ASan reports heap-buffer-overflow */
    char *empty = (char *)malloc(1);
    if (!empty) {
        perror("malloc");
        return 1;
    }
    empty[0] = '\0';

    /* This triggers the bug: opt_path_end returns empty-1, then strrchr reads OOB */
    char *name = opt_progname(empty);

    /* Use the result to prevent over-optimization */
    printf("progname: %s\n", name);

    free(empty);
    return 0;
}
