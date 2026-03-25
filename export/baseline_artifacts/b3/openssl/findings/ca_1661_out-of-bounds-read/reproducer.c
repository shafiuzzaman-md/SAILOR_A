#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdint.h>

/* Minimal stand-ins for OpenSSL types used in the vulnerable call */
typedef struct bio_st BIO;
struct bio_st { int dummy; };

typedef struct asn1_string_st {
    int length;
    int type;
    unsigned char *data;
    long flags;
} ASN1_STRING;

/* Global like apps/bio_err */
static BIO *bio_err = NULL;

/*
 * Stub BIO_printf that intentionally reads char* arguments until a NUL byte
 * (like printf with %s would). Because this function is compiled with
 * -fsanitize=address, the character-by-character loads are instrumented and
 * an out-of-bounds read will be reliably detected as soon as it crosses the
 * allocation boundary.
 */
int BIO_printf(BIO *bp, const char *fmt, ...)
{
    (void)bp; /* unused in stub */

    va_list ap;
    va_start(ap, fmt);

    /* Expected order for the specific format used in apps/ca.c:1661 */
    const char *cv_name = va_arg(ap, const char *);   /* %s (policy field name) */
    const char *ca_str  = va_arg(ap, const char *);   /* %s (issuer/CA ASN1_STRING->data) */
    const char *req_str = va_arg(ap, const char *);   /* %s (request ASN1_STRING->data) */

    /* Print some context (not necessary for the bug) */
    fputs("[BIO_printf stub] The ", stderr);
    fputs(cv_name ? cv_name : "(null)", stderr);
    fputs(" field is different between\nCA certificate (", stderr);

    /* Intentionally read char-by-char until NUL terminator (like %s).
     * If ca_str is not NUL-terminated within its allocated object,
     * this will read into redzone and ASan will report a heap-buffer-overflow. */
    if (ca_str == NULL) {
        fputs("NULL", stderr);
    } else {
        const char *p = ca_str;
        while (*p) { /* OOB read happens here when *p crosses the buffer end */
            fputc(*p, stderr);
            p++;
        }
    }

    fputs(") and the request (", stderr);

    if (req_str == NULL) {
        fputs("NULL", stderr);
    } else {
        const char *p = req_str;
        while (*p) { /* OOB read happens here as well */
            fputc(*p, stderr);
            p++;
        }
    }

    fputs(")\n", stderr);

    va_end(ap);
    return 0;
}

/*
 * Minimal reproducer for the apps/ca.c:certify() vulnerable line.
 * We mimic a policy 'match' failure path and directly hit the BIO_printf
 * that prints ASN1_STRING->data with %s while not ensuring NUL termination.
 */
static void certify(void)
{
    /* Craft two ASN1_STRINGs whose ->data buffers are not NUL-terminated */
    ASN1_STRING *str  = (ASN1_STRING *)calloc(1, sizeof(*str));   /* request value */
    ASN1_STRING *str2 = (ASN1_STRING *)calloc(1, sizeof(*str2));  /* CA value */

    /* Allocate small buffers with no trailing NUL to force OOB read */
    size_t n1 = 7;  /* request len */
    size_t n2 = 5;  /* CA len */

    unsigned char *d1 = (unsigned char *)malloc(n1);
    unsigned char *d2 = (unsigned char *)malloc(n2);

    if (!str || !str2 || !d1 || !d2) {
        fprintf(stderr, "alloc failed\n");
        exit(1);
    }

    /* Fill with non-zero bytes so no NUL is present in-buffer */
    for (size_t i = 0; i < n1; i++) d1[i] = (unsigned char)('A' + (int)(i % 26));
    for (size_t i = 0; i < n2; i++) d2[i] = (unsigned char)('a' + (int)(i % 26));

    str->data  = d1; str->length  = (int)n1;
    str2->data = d2; str2->length = (int)n2;

    const char *cv_name = "CN"; /* policy field name */

    /* This call mirrors the vulnerable use in apps/ca.c:1661 */
    BIO_printf(bio_err,
               "The %s field is different between\nCA certificate (%s) and the request (%s)\n",
               cv_name,
               (str2 == NULL) ? "NULL" : (char *)str2->data,
               (str  == NULL) ? "NULL" : (char *)str->data);

    /* Clean up (won't affect the already-triggered read) */
    free(d1);
    free(d2);
    free(str);
    free(str2);
}

int main(void)
{
    /* Initialize dummy BIO sink */
    bio_err = (BIO *)malloc(sizeof(*bio_err));
    if (!bio_err) {
        fprintf(stderr, "bio_err alloc failed\n");
        return 1;
    }

    certify();

    free(bio_err);
    return 0;
}
