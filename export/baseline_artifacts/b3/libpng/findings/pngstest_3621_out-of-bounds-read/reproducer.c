#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 Reproducer for pngstest.c --tmpfile handling bug (CWE-125, OOB read)
 The original code does:
   if (strlen(argv[++c]) >= sizeof tmpf) ...
   strncpy(tmpf, argv[c], sizeof(tmpf)-1);  // if len == sizeof(tmpf)-1 -> no NUL terminator
   ... later uses tmpf as a C string
 This reproducer mimics that logic and then uses tmpf as a C string to
 trigger an out-of-bounds read under ASan.
*/

#define TMPF_SIZE 16  /* Size of tmpf[] buffer; exact value in pngstest.c is not required to reproduce */

static size_t manual_strlen(const char *s)
{
    /* Manual strlen to ensure ASan-instrumented byte-by-byte reads */
    size_t i = 0;
    while (s[i] != '\0')
        i++;
    return i;
}

static int pngstest_like_main(int argc, char **argv)
{
    /* Simulate only the relevant --tmpfile handling from pngstest.c */
    char tmpf[TMPF_SIZE];

    if (argc < 3 || strcmp(argv[1], "--tmpfile") != 0) {
        fprintf(stderr, "usage: %s --tmpfile <prefix>\n", argv[0]);
        return 1;
    }

    /* Fill tmpf with non-zero bytes so lack of NUL is guaranteed */
    memset(tmpf, 'A', sizeof(tmpf));

    /* Vulnerable logic: reject only strictly >= sizeof(tmpf),
     * then copy up to sizeof(tmpf)-1 bytes WITHOUT appending NUL. */
    if (strlen(argv[2]) >= sizeof(tmpf)) {
        fprintf(stderr, "%s: %s is too long for a temp file prefix\n", argv[0], argv[2]);
        return 99;
    }

    /* If argv[2] length == sizeof(tmpf)-1, tmpf is NOT NUL-terminated */
    strncpy(tmpf, argv[2], sizeof(tmpf) - 1);

    /* Subsequent use of tmpf as a C string causes an OOB read.
     * We trigger it deterministically by scanning for a NUL byte. */
    size_t n1 = manual_strlen(tmpf);            /* ASan should flag OOB here */
    size_t n2 = strlen(tmpf);                   /* And/or here via interceptor */

    /* Prevent optimizations and show that we used the values */
    printf("manual_strlen(tmpf)=%zu, strlen(tmpf)=%zu\n", n1, n2);

    return 0;
}

int main(void)
{
    /* Craft argv so that prefix length is exactly TMPF_SIZE-1. */
    static char prefix[TMPF_SIZE];
    /* Fill with 'B' characters; ensure the source string length is TMPF_SIZE-1 */
    memset(prefix, 'B', TMPF_SIZE - 1);
    prefix[TMPF_SIZE - 1] = '\0';

    char *argv[] = { (char*)"reproducer", (char*)"--tmpfile", prefix, NULL };
    int argc = 3;

    return pngstest_like_main(argc, argv);
}
