#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

/* Minimal stand-ins for OpenSSL types and APIs used by the vulnerable code */

typedef struct x509_st {
    int dummy;
} X509;

/* Model the OPENSSL STACK_OF(X509) type */
struct stack_st_X509 {
    int dummy;
};
#define STACK_OF(type) struct stack_st_##type

/* Flags used by X509_add_cert */
#define X509_ADD_FLAG_UP_REF 1

/* Stubs mirroring the OpenSSL API surface used by save_cert_or_delete */
static STACK_OF(X509) *sk_X509_new_null(void)
{
    /* Simulate allocation failure to trigger the bug: return NULL */
    return NULL;
}

static void sk_X509_free(STACK_OF(X509) *sk)
{
    (void)sk; /* nothing to free in stub */
}

/* This function will dereference the 'sk' argument (stack) to emulate OpenSSL behavior.
 * Passing a NULL 'sk' will cause a null-pointer-dereference just like the real bug. */
int X509_add_cert(STACK_OF(X509) *sk, X509 *cert, int flags)
{
    (void)cert;
    (void)flags;
    /* Intentional dereference of potentially NULL 'sk' to reproduce the crash */
    int touch = sk->dummy; /* boom if sk == NULL */
    (void)touch;
    return 1;
}

/* save_free_certs is called after X509_add_cert() on success; not reached in this PoC */
static int save_free_certs(STACK_OF(X509) *certs, const char *file, const char *desc)
{
    (void)certs;
    (void)file;
    (void)desc;
    return 1;
}

/* delete_file is called only when cert == NULL branch is taken; we still stub it */
static int delete_file(const char *file, const char *desc)
{
    (void)file;
    (void)desc;
    return 1;
}

/* BIO_snprintf used in the other branch; provide a simple wrapper */
int BIO_snprintf(char *buf, size_t n, const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    int ret = vsnprintf(buf, n, format, ap);
    va_end(ap);
    return ret;
}

/* Vulnerable function extracted/simplified from apps/cmp.c */
static int save_cert_or_delete(X509 *cert, const char *file, const char *desc)
{
    if (file == NULL)
        return 1;
    if (cert == NULL) {
        char desc_cert[80];

        BIO_snprintf(desc_cert, sizeof(desc_cert), "%s certificate", desc);
        return delete_file(file, desc_cert);
    } else {
        STACK_OF(X509) *certs = sk_X509_new_null();

        /* BUG: certs may be NULL on allocation failure; not checked before use */
        if (!X509_add_cert(certs, cert, X509_ADD_FLAG_UP_REF)) {
            sk_X509_free(certs);
            return 0;
        }
        return save_free_certs(certs, file, desc) >= 0;
    }
}

/* Minimal constructor for a dummy X509 object */
static X509 *X509_new(void)
{
    X509 *x = (X509 *)malloc(sizeof(*x));
    if (x)
        x->dummy = 0x1234;
    return x;
}

int main(void)
{
    /* Create a non-NULL cert and provide a non-NULL file path to hit the buggy branch */
    X509 *cert = X509_new();
    if (!cert) {
        fprintf(stderr, "Failed to allocate dummy X509 cert\n");
        return 1;
    }

    /* This call will attempt to add cert to a NULL stack (sk_X509_new_null returns NULL),
       triggering a null-pointer dereference inside X509_add_cert. */
    (void)save_cert_or_delete(cert, "output.pem", "test");

    /* Not reached due to crash */
    free(cert);
    return 0;
}
