#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* Undefine potential ctype macros so our interposed functions are used */
#undef tolower
#undef toupper

/* Interpose tolower to force an ASan-detectable OOB read when called with
 * a negative value (UB in the original code). This makes the undefined
 * behavior visible even if the system libc would not crash. */
int tolower(int c) {
    static unsigned char *table;
    if (!table) {
        table = (unsigned char *)malloc(256);
        if (!table) exit(1);
        for (int i = 0; i < 256; ++i) table[i] = (unsigned char)i;
    }
    /* Intentional: if c < 0 or c > 255, this indexes OOB. This simulates
       libc ctype table OOB caused by passing a negative char to tolower. */
    return table[c];
}

/* Provide a benign toupper to satisfy references in chstore when flags=1 */
int toupper(int c) {
    return c;
}

/* States, matching the original */
#define IN_WHITE 0
#define IN_TOKEN 1
#define IN_QUOTE 2
#define IN_OZONE 3

/* Globals as in contrib/iptcutil/iptcutil.c */
int _p_state;         /* current state   */
unsigned _p_flag;     /* option flag     */
char _p_curquote;     /* current quote   */
int _p_tokpos;        /* current token pos */

/* sindex from original (not strictly needed for this stub) */
static int sindex(char ch, const char *string)
{
    const char *cp;
    for (cp = string; *cp; ++cp)
        if (ch == *cp)
            return (int)(cp - string);
    return -1;
}

/* Vulnerable chstore from contrib/iptcutil/iptcutil.c */
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
                    c = (char)toupper((int)ch);
                    break;

                case 2: /* convert to lower */
                    /* BUG: passing possibly-negative (signed) char to tolower */
                    c = (char)tolower((int)ch);
                    break;

                default: /* use as is */
                    c = ch;
                    break;
            }
        string[_p_tokpos++] = c;
    }
}

/* Minimal tokenizer stub that drives chstore along the non-quoted path.
 * It sets _p_flag=inflag and feeds bytes from 'line' into chstore. */
int tokenizer(unsigned inflag, char *token, int tokmax, char *line,
              const char *white, const char *brkchar, const char *quote,
              char eschar, char *brkused, int *next, char *quoted)
{
    (void)white; (void)brkchar; (void)quote; (void)eschar;
    *brkused = 0;
    *quoted = 0;

    _p_state = IN_TOKEN; /* ensure we go through the case-conversion path */
    _p_curquote = 0;
    _p_flag = inflag;    /* propagate flags to chstore */
    _p_tokpos = 0;

    for (; line[*next]; ++(*next)) {
        /* Feed each byte to chstore; for inflag bit 2, chstore will call tolower */
        chstore(token, tokmax, line[*next]);
    }
    token[_p_tokpos] = '\0';
    return 0;
}

int main(void)
{
    /* Craft input with a byte >= 0x80. On platforms where 'char' is signed
     * (common on x86/x86_64), this makes 'ch' negative inside chstore. */
    char line[2];
    line[0] = (char)0xE1; /* non-ASCII byte; negative when char is signed */
    line[1] = '\0';

    char token[16];
    int next = 0;
    char brkused = 0;
    char quoted = 0;

    /* inflag bit 2 => lower-case conversion path, which calls tolower */
    unsigned inflag = 2;

    /* Call tokenizer to reach chstore with the crafted input */
    tokenizer(inflag, token, (int)sizeof(token), line,
              "", "", "", '\\', &brkused, &next, &quoted);

    /* Prevent optimizing everything away */
    printf("Token: %s\n", token);
    return 0;
}
