#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>

/* Replicated state and flags from contrib/iptcutil/iptcutil.c */
#define IN_WHITE 0
#define IN_TOKEN 1
#define IN_QUOTE 2
#define IN_OZONE 3

int _p_state;     /* current state    */
unsigned _p_flag; /* option flag      */
char _p_curquote; /* current quote char */
int _p_tokpos;    /* current token pos  */

/* sindex helper (not critical for the trigger, included for completeness) */
static int sindex(char ch, const char *string)
{
    const char *cp;
    for (cp = string; *cp; ++cp)
        if (ch == *cp)
            return (int)(cp - string);
    return -1;
}

/* We intentionally override toupper to make out-of-bounds reads visible to ASan.
   This mimics a libc implementation that indexes into a 256-entry table. */
#undef toupper
#undef tolower

static unsigned char asan_upper_table[256];
static int asan_upper_init = 0;

static void asan_upper_init_table(void) {
    for (int i = 0; i < 256; i++) {
        asan_upper_table[i] = (unsigned char)i;
        if (i >= 'a' && i <= 'z') {
            asan_upper_table[i] = (unsigned char)(i - ('a' - 'A'));
        }
    }
    asan_upper_init = 1;
}

int toupper(int x) {
    if (!asan_upper_init) asan_upper_init_table();
    if (x == -1) return x; /* EOF passthrough */
    /* Intentional bug emulation: no range check, directly index the table.
       If x is negative (e.g., when caller passes a negative char), this is
       a read before the start of asan_upper_table, which ASan reports. */
    return asan_upper_table[x];
}

/* Vulnerable chstore from iptcutil.c: uses toupper/tolower on (int)ch where
   ch is a possibly-negative 'char'. */
static void chstore(char *string, int max, char ch)
{
    char c;
    if (_p_tokpos >= 0 && _p_tokpos < max - 1)
    {
        if (_p_state == IN_QUOTE)
            c = ch;
        else
            switch (_p_flag & 3)
            {
                case 1: /* convert to upper */
                    c = (char)toupper((int)ch); /* UB when ch is negative */
                    break;

                case 2: /* convert to lower */
                    c = (char)tolower((int)ch);
                    break;

                default: /* use as is */
                    c = ch;
                    break;
            }
        string[_p_tokpos++] = c;
    }
    return;
}

/* Minimal tokenizer that reaches chstore in the same way as the real code when
   inflag&3 == 1 (uppercase conversion) and input has a high-bit byte. */
static int tokenizer(unsigned inflag, char *token, int tokmax, char *line,
                     const char *white, const char *brkchar, const char *quote,
                     char eschar, char *brkused, int *next, char *quoted)
{
    (void)white; (void)brkchar; (void)quote; (void)eschar; (void)brkused; (void)quoted;

    _p_state = IN_WHITE;
    _p_curquote = 0;
    _p_flag = inflag;

    _p_tokpos = 0;

    /* Simulate entering IN_TOKEN immediately and storing the current char */
    char c = line[*next];
    if (!c) return 1; /* end of line */
    _p_state = IN_TOKEN;
    chstore(token, tokmax, c);
    (*next)++;
    token[_p_tokpos] = '\0';
    return 0;
}

int main(void)
{
    /* Craft an input line containing a high-bit byte (>= 0x80).
       On platforms where 'char' is signed (common on x86_64 Linux), this
       value becomes negative when stored in 'char', triggering the UB path. */
    char line[] = { (char)0x80, 0 };

    char token[8];
    int next = 0;
    char brkused = 0;
    char quoted = 0;

    /* inflag bit 0..1 == 1 -> convert to upper */
    unsigned inflag = 1u;

    /* These strings are not used by our minimal tokenizer but provided to match signature */
    const char *white = " \t";
    const char *brkchar = ",;";
    const char *quote = "\"'";
    char eschar = '\\';

    /* Call tokenizer to reach chstore and ultimately toupper with a negative value. */
    (void)tokenizer(inflag, token, (int)sizeof(token), line,
                    white, brkchar, quote, eschar, &brkused, &next, &quoted);

    /* Prevent optimizing away */
    fprintf(stderr, "token: %s\n", token);

    return 0;
}
