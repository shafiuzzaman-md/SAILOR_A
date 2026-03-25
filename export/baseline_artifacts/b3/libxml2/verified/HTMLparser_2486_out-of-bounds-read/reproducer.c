#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdint.h>

/* Minimal stand-ins for libxml2 types/macros */
typedef unsigned char xmlChar;

#define ENT_F_SEMICOLON 0x40
#define ENT_F_SUBTABLE  0x80
#define ENT_F_ALL       (ENT_F_SEMICOLON | ENT_F_SUBTABLE)

#define IS_ASCII_LETTER(c) ((((c) >= 'A') && ((c) <= 'Z')) || (((c) >= 'a') && ((c) <= 'z')))
#define IS_ALNUM(c) ((((c) >= '0') && ((c) <= '9')) || IS_ASCII_LETTER(c))

/* Minimal global tables required by htmlFindEntityPrefix */
static unsigned char htmlEntAlpha[64 * 3];
static unsigned int  htmlEntValues[1] = { 0 };
/* bytes layout: [len|flags, char1, char2, ...] */
static unsigned char htmlEntStrings[] = {
    2,    /* len == 2, no flags set */
    'b',  /* bytes[1] == 'b' -> must match string[1] */
    'x'   /* bytes[2] arbitrary; strncmp will try to read 1 byte here */
};

/* Vulnerable function (reduced to the relevant logic) */
static const xmlChar *htmlFindEntityPrefix(const xmlChar *string, size_t slen, int isAttr) {
    unsigned left, right;
    const xmlChar *match = NULL;
    int first = string[0];
    size_t matchLen = 0;
    size_t soff = 1;

    (void)isAttr; /* not relevant for triggering the bug */

    if (slen < 2)
        return NULL;
    if (!IS_ASCII_LETTER(first))
        return NULL;

    /* Map first char into bucket [0..63] */
    first &= 63;
    left  = (unsigned)(htmlEntAlpha[first*3] | (htmlEntAlpha[first*3+1] << 8));
    right = left + htmlEntAlpha[first*3+2];

    while (left < right) {
        const xmlChar *bytes;
        unsigned mid;
        size_t len;
        int cmp;

        mid = left + (right - left) / 2;
        bytes = htmlEntStrings + htmlEntValues[mid];
        len = (size_t)(bytes[0] & ~ENT_F_ALL);

        cmp = (int)string[soff] - (int)bytes[1];

        if (cmp == 0) {
            if (slen < len) {
                cmp = strncmp((const char *) string + soff + 1,
                              (const char *) bytes + 2,
                              slen - 1);
                if (cmp == 0)
                    break;
            } else {
                /* Vulnerable call when slen == len (reads 1 past end of string) */
                cmp = strncmp((const char *) string + soff + 1,
                              (const char *) bytes + 2,
                              len - 1);
            }
        }

        if (cmp < 0) {
            right = mid;
        } else if (cmp > 0) {
            left = mid + 1;
        } else {
            int term = (soff + len < slen) ? string[soff + len] : 0;
            int isAlnum = IS_ALNUM(term);
            int isTerm = ((term == ';') ||
                          ((bytes[0] & ENT_F_SEMICOLON) && ((!isAttr) || ((!isAlnum) && (term != '=')))));
            if (isTerm) {
                match = bytes + len + 1;
                matchLen = soff + len;
                if (term == ';')
                    matchLen += 1;
            }
            break;
        }
    }

    (void)matchLen; /* silence unused warning */
    return match;
}

int main(void) {
    /* Prepare the bucket for first letter 'a' (97 & 63 == 33) */
    unsigned bucket = ((unsigned)('a') & 63u) * 3u;
    /* left = 0, right = 1 -> exactly one entry */
    htmlEntAlpha[bucket + 0] = 0;  /* low byte of left */
    htmlEntAlpha[bucket + 1] = 0;  /* high byte of left */
    htmlEntAlpha[bucket + 2] = 1;  /* count */

    /* Craft input string of length 2: "ab" (no NUL terminator on purpose) */
    size_t slen = 2;
    unsigned char *s = (unsigned char *)malloc(slen);
    if (!s) return 1;
    s[0] = 'a';
    s[1] = 'b';

    /* This call will execute the vulnerable strncmp with n = len-1 = 1,
       reading from s + 2, which is one byte past the allocated buffer. */
    const xmlChar *res = htmlFindEntityPrefix(s, slen, 0);

    /* Prevent optimizing away */
    printf("result ptr: %p\n", (void*)res);

    free(s);
    return 0;
}