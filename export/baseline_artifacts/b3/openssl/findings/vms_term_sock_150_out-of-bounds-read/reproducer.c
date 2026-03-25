#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>

/*
 * Minimal stub for the OPENSSL_strcasecmp used by the vulnerable code.
 * To deterministically expose the bug (reading past a non-NUL-terminated
 * buffer), this implementation scans the first argument until it finds a
 * NUL byte. If the buffer lacks a NUL within its bounds, this will read
 * past the end of the buffer and trigger AddressSanitizer.
 */
static int OPENSSL_strcasecmp(const char *a, const char *b) {
    (void)b; /* unused in this stub */
    const unsigned char *p = (const unsigned char *)a;
    while (*p) {            /* keep scanning until we see a NUL */
        /* tolower to mimic strcasecmp-like access semantics */
        (void)tolower(*p);
        p++;
    }
    /* Non-zero return to match while (.. != 0) logic of the original */
    return 1;
}

static void LogMessage(char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stdout, fmt, ap);
    fputc('\n', stdout);
    va_end(ap);
}

int main(void) {
    /* Replicate the vulnerable pattern from apps/lib/vms_term_sock.c (TERM_SOCK_TEST) */
    char TermBuff[80];

    /* The original bug is that TermBuff is uninitialized before being passed
     * to OPENSSL_strcasecmp. To make the out-of-bounds read deterministic
     * under ASan, we explicitly fill it with non-NUL bytes so there is no
     * terminator within the 80-byte buffer. This models an uninitialized
     * buffer that happens to contain no NULs. */
    memset(TermBuff, 'A', sizeof(TermBuff));

    LogMessage("Enter 'q' or 'Q' to quit ...");

    /* This while condition calls OPENSSL_strcasecmp on a non-NUL-terminated
     * TermBuff. Our OPENSSL_strcasecmp scans until it finds a NUL, so it will
     * read past the end of TermBuff and AddressSanitizer will report an
     * out-of-bounds read at this point. */
    while (OPENSSL_strcasecmp(TermBuff, "Q")) {
        /* We should not reach this block before ASan reports the OOB read. */
        break;
    }

    return 0;
}
