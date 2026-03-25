#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

/* Minimal stub type definitions to mirror OpenSSL types */
typedef struct bio_st { int dummy; } BIO;
typedef struct X509_st { int dummy; } X509;
typedef struct X509_NAME_st { int dummy; } X509_NAME;

/* Global error BIO used by apps.c */
BIO *bio_err = NULL;

/* Stub: OPENSSL_free - just use free for this reproducer */
void OPENSSL_free(void *p) {
    free(p);
}

/* Stub: X509_get_subject_name - return any non-NULL pointer */
X509_NAME *X509_get_subject_name(X509 *cert) {
    (void)cert;
    static X509_NAME dummy_name;
    return &dummy_name; /* non-NULL */
}

/* Stub: X509_NAME_oneline - force failure by returning NULL */
char *X509_NAME_oneline(X509_NAME *a, char *buf, int size) {
    (void)a; (void)buf; (void)size;
    return NULL; /* Simulate allocation/encoding failure */
}

/* Stub: BIO_printf - deliberately dereference the %s argument to mimic
 * the vulnerable behavior when a NULL string is passed. This ensures a
 * deterministic NULL pointer dereference regardless of libc behavior. */
int BIO_printf(BIO *bp, const char *format, ...) {
    (void)bp;
    va_list args;
    va_start(args, format);
    const char *uri  = va_arg(args, const char *);
    const char *subj = va_arg(args, const char *);
    const char *msg  = va_arg(args, const char *);

    /* Force a NULL pointer dereference exactly as would happen when
     * using "%s" with a NULL pointer in some printf implementations. */
    volatile char c = subj[0]; /* Crash here when subj == NULL */
    (void)c;

    /* Unreachable if crash occurs; present to satisfy the signature */
    int ret = fprintf(stderr, format, uri, subj, msg);
    va_end(args);
    return ret;
}

/* Vulnerable function copied from apps/lib/apps.c (line numbers removed) */
static void warn_cert_msg(const char *uri, X509 *cert, const char *msg)
{
    char *subj = X509_NAME_oneline(X509_get_subject_name(cert), NULL, 0);

    /* Vulnerable call: subj can be NULL, leading to NULL deref via BIO_printf */
    BIO_printf(bio_err, "Warning: certificate from '%s' with subject '%s' %s\n",
               uri, subj, msg);
    OPENSSL_free(subj);
}

int main(void) {
    /* Initialize a dummy BIO to mimic bio_err presence */
    static BIO dummy_bio;
    bio_err = &dummy_bio;

    /* Dummy certificate object */
    X509 cert;

    /* Call the vulnerable path: X509_NAME_oneline returns NULL, which is
     * then passed to BIO_printf with "%s" -> triggers NULL deref. */
    warn_cert_msg("test://uri", &cert, "triggering NULL deref");

    return 0; /* Not reached if the bug is triggered */
}
