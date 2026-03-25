#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Minimal stand-in for the Windows program name used in rpng2-win.c */
#define PROGNAME "rpng2-win"

/*
 * This function reproduces the vulnerable tokenization logic from
 * contrib/gregbook/rpng2-win.c:WinMain. It builds an argv[1024] on the stack
 * and then overflows it by processing too many space-separated tokens from
 * the command line string.
 */
__attribute__((noinline)) static void vulnerable_tokenizer(char *cmd)
{
    char *argv[1024];   /* fixed-size stack array, like in rpng2-win.c */
    int argc = 0;
    char *p, *q;

    /* Mimic the code around lines 368-386 */
    argv[argc++] = (char *)PROGNAME;  /* consumes one slot */
    p = cmd;
    for (;;) {
        if (*p == ' ')
            while (*++p == ' ')
                ;
        if (*p == '\0')
            break;
        argv[argc++] = q = p;          /* may overflow when too many tokens */
        while (*q && *q != ' ')
            ++q;
        if (*q == '\0')
            break;
        *q = '\0';
        p = q + 1;
    }
    /* This write matches line 386 and will go out-of-bounds when argc == 1024 */
    argv[argc] = NULL;                 /* definite OOB when exactly 1023 tokens */

    /* Keep the local argv "live" to prevent over-aggressive elimination */
    if (argc > 0 && argv[0]) {
        volatile char c = argv[0][0];
        (void)c;
    }
}

int main(void)
{
    /* Build a command line with exactly 1023 short tokens ("a"), space-separated.
     * After inserting PROGNAME first, the next 1023 tokens fill the 1024 entries.
     * The subsequent argv[argc] = NULL write overflows the stack array by 1. */
    const int tokens = 1023; /* triggers the overflow at the terminator write */

    size_t len = (size_t)tokens               /* characters */
               + (size_t)(tokens - 1)         /* spaces */
               + 1;                           /* NUL */

    char *cmd = (char *)malloc(len);
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

    vulnerable_tokenizer(cmd);

    free(cmd);
    return 0;
}
