#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Minimal stand-in for the global buffer used by opt_progname */
static char prog[64];

/* Vulnerable implementation from the non-Windows, non-VMS code path */
const char *opt_path_end(const char *filename)
{
    const char *p;

    /* For empty filename, this pre-decrements p to filename-1 */
    for (p = filename + strlen(filename); --p > filename;)
        if (*p == '/') {
            p++;
            break;
        }
    return p;
}

char *opt_progname(const char *argv0)
{
    const char *p;

    p = opt_path_end(argv0);
    if (prog != p)
        /* p may point to one byte before argv0 when argv0 is "" */
        strncpy(prog, p, sizeof(prog) - 1);
    prog[sizeof(prog) - 1] = '\0';
    return prog;
}

int main(void)
{
    /* Allocate a 1-byte heap buffer for the empty string so ASan redzones catch the OOB read */
    char *empty = (char *)malloc(1);
    if (!empty) {
        perror("malloc");
        return 1;
    }
    empty[0] = '\0';

    /* Triggers: opt_path_end(empty) returns empty-1, so strncpy reads from redzone */
    char *out = opt_progname(empty);

    /* Use the result to prevent it from being optimized away */
    printf("prog: %s\n", out);

    /* No need to free 'empty' for this repro; program exits immediately */
    return 0;
}
