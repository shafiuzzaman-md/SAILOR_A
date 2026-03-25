#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdint.h>

/* Minimal stub types to mirror OpenSSL structures */
typedef struct ssl_ctx_st { int dummy; } SSL_CTX;
typedef struct ssl_st { int dummy; } SSL;
typedef struct ssl_cipher_st { int dummy; } SSL_CIPHER;

typedef struct stack_st_SSL_CIPHER {
    int num; /* deliberately accessed without NULL check to emulate bug */
} STACK_OF_SSL_CIPHER;

/* Global BIO-like placeholders */
static void *bio_out = NULL;
static void *bio_err = NULL;

/* Stubbed BIO I/O */
static int BIO_printf(void *bio, const char *fmt, ...) {
    (void)bio; /* ignore */
    va_list ap;
    va_start(ap, fmt);
    int r = vfprintf(stdout, fmt, ap);
    va_end(ap);
    return r;
}

static int BIO_puts(void *bio, const char *s) {
    (void)bio; /* ignore */
    return fputs(s, stdout);
}

/* Stubbed OpenSSL API */
static SSL_CTX *SSL_CTX_new(void) {
    return (SSL_CTX *)calloc(1, sizeof(SSL_CTX));
}

static void SSL_CTX_free(SSL_CTX *ctx) {
    free(ctx);
}

static SSL *SSL_new(SSL_CTX *ctx) {
    (void)ctx;
    return (SSL *)calloc(1, sizeof(SSL));
}

static void SSL_free(SSL *ssl) {
    free(ssl);
}

static int SSL_CTX_set_ciphersuites(SSL_CTX *ctx, const char *suites) {
    (void)ctx; (void)suites; return 1; /* success */
}

static int SSL_CTX_set_cipher_list(SSL_CTX *ctx, const char *list) {
    (void)ctx; (void)list; return 1; /* success */
}

static STACK_OF_SSL_CIPHER *SSL_get1_supported_ciphers(SSL *ssl) {
    (void)ssl;
    /* Return NULL to model failure/no ciphers available */
    return NULL;
}

static STACK_OF_SSL_CIPHER *SSL_get_ciphers(SSL *ssl) {
    (void)ssl;
    /* Return NULL to trigger the NULL pointer dereference in ciphers_main */
    return NULL;
}

/* OpenSSL stack accessors (intentionally dereference without NULL check) */
static int sk_SSL_CIPHER_num(const STACK_OF_SSL_CIPHER *sk) {
    /* Vulnerable behavior: this will crash if sk == NULL */
    return sk->num;
}

static const SSL_CIPHER *sk_SSL_CIPHER_value(const STACK_OF_SSL_CIPHER *sk, int i) {
    (void)sk; (void)i; return NULL;
}

/* Remaining stubs referenced in the verbose path */
static const char *SSL_CIPHER_get_name(const SSL_CIPHER *c) { (void)c; return "DUMMY"; }
static const char *SSL_CIPHER_standard_name(const SSL_CIPHER *c) { (void)c; return "STD"; }
static unsigned long SSL_CIPHER_get_id(const SSL_CIPHER *c) { (void)c; return 0x03000001UL; }
static const char *SSL_CIPHER_description(const SSL_CIPHER *c, char *buf, size_t len) {
    (void)c; (void)buf; (void)len; return "desc";
}
static int ossl_assert(int cond) { return cond; }
static void sk_SSL_CIPHER_free(STACK_OF_SSL_CIPHER *sk) { free(sk); }
static void ERR_print_errors(void *bio) { (void)bio; }

/* Reimplementation of the vulnerable function logic focusing on the buggy part */
static int ciphers_main(void) {
    int ret = 1;
    SSL_CTX *ctx = NULL;
    SSL *ssl = NULL;
    int verbose = 1;      /* Force verbose path to hit line 246 analog */
    int use_supported = 0;/* Choose SSL_get_ciphers() path */
    int stdname = 0;
    int Verbose = 0;

    const char *ciphersuites = NULL;
    const char *ciphers = NULL;

    STACK_OF_SSL_CIPHER *sk = NULL;
    int i;
    char buf[256];

    ctx = SSL_CTX_new();
    if (ctx == NULL)
        goto err;

    if (ciphersuites != NULL && !SSL_CTX_set_ciphersuites(ctx, ciphersuites)) {
        BIO_printf(bio_err, "Error setting TLSv1.3 ciphersuites\n");
        goto err;
    }

    if (ciphers != NULL) {
        if (!SSL_CTX_set_cipher_list(ctx, ciphers)) {
            BIO_printf(bio_err, "Error in cipher list\n");
            goto err;
        }
    }

    ssl = SSL_new(ctx);
    if (ssl == NULL)
        goto err;

    if (use_supported)
        sk = SSL_get1_supported_ciphers(ssl);
    else
        sk = SSL_get_ciphers(ssl);

    if (!verbose) {
        for (i = 0; i < sk_SSL_CIPHER_num(sk); i++) { /* not taken */
            const SSL_CIPHER *c = sk_SSL_CIPHER_value(sk, i);
            if (!ossl_assert(c != NULL))
                continue;
            const char *p = SSL_CIPHER_get_name(c);
            if (p == NULL)
                break;
            if (i != 0)
                BIO_printf(bio_out, ":");
            BIO_printf(bio_out, "%s", p);
        }
        BIO_printf(bio_out, "\n");
    } else {
        /* This call will dereference NULL inside sk_SSL_CIPHER_num() and crash */
        for (i = 0; i < sk_SSL_CIPHER_num(sk); i++) {
            const SSL_CIPHER *c;
            c = sk_SSL_CIPHER_value(sk, i);
            if (!ossl_assert(c != NULL))
                continue;
            if (Verbose) {
                unsigned long id = SSL_CIPHER_get_id(c);
                int id0 = (int)(id >> 24);
                int id1 = (int)((id >> 16) & 0xffL);
                int id2 = (int)((id >> 8) & 0xffL);
                int id3 = (int)(id & 0xffL);
                if ((id & 0xff000000L) == 0x03000000L)
                    BIO_printf(bio_out, "          0x%02X,0x%02X - ", id2, id3);
                else
                    BIO_printf(bio_out, "0x%02X,0x%02X,0x%02X,0x%02X - ", id0, id1, id2, id3);
            }
            if (stdname) {
                const char *nm = SSL_CIPHER_standard_name(c);
                if (nm == NULL)
                    nm = "UNKNOWN";
                BIO_printf(bio_out, "%-45s - ", nm);
            }
            BIO_puts(bio_out, SSL_CIPHER_description(c, buf, sizeof(buf)));
        }
    }

    ret = 0;
    goto end;
err:
    ERR_print_errors(bio_err);
end:
    if (use_supported)
        sk_SSL_CIPHER_free(sk);
    SSL_CTX_free(ctx);
    SSL_free(ssl);
    return ret;
}

int main(void) {
    /* Running this will reliably trigger the NULL-pointer dereference
       when evaluating sk_SSL_CIPHER_num(sk) with sk == NULL. */
    return ciphers_main();
}
