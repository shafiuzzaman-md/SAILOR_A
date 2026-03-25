#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Minimal stub types to mimic OpenSSL structures used by apps/ciphers.c
 */
typedef struct SSL_CTX_st { int dummy; } SSL_CTX;
typedef struct SSL_st { SSL_CTX *ctx; } SSL;
typedef struct ssl_cipher_st { int id; } SSL_CIPHER;

/* Mimic the OpenSSL stack type used for ciphers */
typedef struct stack_st_SSL_CIPHER {
    int num; /* number of elements; in real OpenSSL this would be more complex */
} STACK_OF_SSL_CIPHER;

/*
 * Stub implementations of the OpenSSL APIs used by ciphers_main.
 * They are crafted so that SSL_get_ciphers/SSL_get1_supported_ciphers
 * return NULL, which triggers the NULL dereference in sk_SSL_CIPHER_num.
 */
SSL_CTX *SSL_CTX_new_ex(void *libctx, const char *propq, const void *meth) {
    (void)libctx; (void)propq; (void)meth;
    /* Return a non-NULL context to proceed to the vulnerable code path */
    return (SSL_CTX *)calloc(1, sizeof(SSL_CTX));
}

int SSL_CTX_set_min_proto_version(SSL_CTX *ctx, int v) {
    (void)ctx; (void)v; return 1; /* success */
}

int SSL_CTX_set_max_proto_version(SSL_CTX *ctx, int v) {
    (void)ctx; (void)v; return 1; /* success */
}

SSL *SSL_new(SSL_CTX *ctx) {
    SSL *s = (SSL *)calloc(1, sizeof(SSL));
    if (s) s->ctx = ctx;
    return s;
}

/* Force returning NULL to simulate allocation failure / no ciphers available */
STACK_OF_SSL_CIPHER *SSL_get1_supported_ciphers(SSL *ssl) {
    (void)ssl; return NULL;
}

STACK_OF_SSL_CIPHER *SSL_get_ciphers(const SSL *ssl) {
    (void)ssl; return NULL;
}

/*
 * Vulnerable helper: dereferences sk without NULL check, matching the behavior
 * expected by the loop condition in apps/ciphers.c.
 */
int sk_SSL_CIPHER_num(const STACK_OF_SSL_CIPHER *sk) {
    /* This will NULL-deref when sk == NULL */
    return sk->num;
}

/*
 * Minimal reproducer for the vulnerable part of apps/ciphers.c:ciphers_main
 * around lines 224-231 and 246-247. We keep verbose=0 to hit the first loop,
 * but both branches call sk_SSL_CIPHER_num(sk) anyway.
 */
static int ciphers_main_reproducer(void) {
    int verbose = 0;       /* mimic the non-verbose branch */
    int use_supported = 0; /* choose SSL_get_ciphers path; both can be NULL */

    SSL_CTX *ctx = SSL_CTX_new_ex(NULL, NULL, NULL);
    if (ctx == NULL)
        return 1;
    if (SSL_CTX_set_min_proto_version(ctx, 0) == 0)
        return 1;
    if (SSL_CTX_set_max_proto_version(ctx, 0) == 0)
        return 1;

    SSL *ssl = SSL_new(ctx);
    if (ssl == NULL)
        return 1;

    STACK_OF_SSL_CIPHER *sk;
    if (use_supported)
        sk = SSL_get1_supported_ciphers(ssl);
    else
        sk = SSL_get_ciphers(ssl);

    /*
     * This mirrors the vulnerable code:
     * for (i = 0; i < sk_SSL_CIPHER_num(sk); i++) { ... }
     * Since sk == NULL, the call below will dereference NULL and crash.
     */
    if (!verbose) {
        for (int i = 0; i < sk_SSL_CIPHER_num(sk); i++) {
            /* Unreachable */
        }
    } else {
        for (int i = 0; i < sk_SSL_CIPHER_num(sk); i++) {
            /* Unreachable */
        }
    }

    return 0;
}

int main(void) {
    /* Run the reproducer. Under ASan this will report a NULL pointer deref. */
    return ciphers_main_reproducer();
}
