#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

/* Minimal stand-ins for OpenSSL types */
typedef struct evp_pkey_st EVP_PKEY;
typedef struct bio_st BIO;

/* Global error BIO like OpenSSL apps */
BIO *bio_err;

/* Stub: mimics OpenSSL's EVP_PKEY_get_default_digest_name behavior for Ed25519/Ed448
 * Returns 2 and sets md to "UNDEF", which means the digest must be left unspecified. */
int EVP_PKEY_get_default_digest_name(EVP_PKEY *pkey, char *md, size_t md_len) {
    (void)pkey; /* unused in stub */
    if (md_len > 0) {
        /* Set to "UNDEF" as for Ed25519/Ed448 */
        snprintf(md, md_len, "%s", "UNDEF");
    }
    return 2; /* mandatory digest handling */
}

/* Stub for BIO_puts (not used in this path, provided for completeness) */
int BIO_puts(BIO *b, const char *s) {
    (void)b;
    fputs(s, stderr);
    return (int)strlen(s);
}

/* Stub: BIO_printf that will dereference the %s argument, reproducing the NULL deref
 * that occurs when passing a NULL string to a %s formatter. */
int BIO_printf(BIO *b, const char *fmt, ...) {
    (void)b;
    va_list ap;
    va_start(ap, fmt);
    /* We know the vulnerable call is BIO_printf(bio_err, "message digest is %s\n", dgst); */
    const char *s = va_arg(ap, const char *);
    /* Force a dereference to trigger ASan null-pointer-dereference when s == NULL */
    volatile char ch = s[0];
    (void)ch;
    va_end(ap);
    /* If not crashed, print to show flow (not expected to reach here in reproducer) */
    fprintf(stderr, fmt, s);
    return 0;
}

/* This function mimics the vulnerable portion of apps/ca.c:ca_main */
static void trigger_vulnerable_path(void) {
    EVP_PKEY *pkey = (EVP_PKEY*)0x1; /* dummy non-NULL key */
    char def_dgst[80];
    char *dgst = NULL; /* as if user didn't specify -md */
    int def_ret;

    /* apps/ca.c lines 836-848 equivalent */
    def_ret = EVP_PKEY_get_default_digest_name(pkey, def_dgst, sizeof(def_dgst));
    if (def_ret == 2 && strcmp(def_dgst, "UNDEF") == 0) {
        /* For Ed25519/Ed448, this path sets dgst to NULL */
        dgst = NULL;
    } else if (dgst == NULL /* && lookup_conf(...) == NULL && strcmp(def_dgst, "UNDEF") != 0 */) {
        /* Not taken in this reproducer */
        ;
    } else {
        /* Not taken in this reproducer */
        ;
    }

    /* apps/ca.c lines 862-871 equivalent: req set and verbose set */
    int req = 1;
    int verbose = 1;
    if (req) {
        if (verbose)
            /* This call passes a NULL string to %s, triggering the crash */
            BIO_printf(bio_err, "message digest is %s\n", dgst);
    }
}

int main(void) {
    /* Set bio_err to a non-NULL dummy; our BIO_printf ignores it */
    bio_err = (BIO*)0x1;

    /* Trigger the vulnerable code path */
    trigger_vulnerable_path();

    return 0;
}
