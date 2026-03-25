#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>
#include <stddef.h>

/* Minimal stub type definitions to mimic OpenSSL structures */
typedef struct evp_pkey_st { int dummy; } EVP_PKEY;
typedef struct evp_cipher_st { int dummy; } EVP_CIPHER;
typedef struct bio_st { FILE *fp; } BIO;
typedef struct ossl_encoder_ctx_st { int dummy; } OSSL_ENCODER_CTX;

enum {
    OSSL_KEYMGMT_SELECT_DOMAIN_PARAMETERS = 0x0001,
    OSSL_KEYMGMT_SELECT_PUBLIC_KEY        = 0x0002,
    OSSL_KEYMGMT_SELECT_ALL               = 0xFFFF
};

/* Stubs for the OpenSSL APIs referenced by apps/ec.c */
OSSL_ENCODER_CTX *OSSL_ENCODER_CTX_new_for_pkey(EVP_PKEY *pkey, int selection,
                                                const char *output_type,
                                                const char *output_structure,
                                                const char *propq)
{
    (void)pkey; (void)selection; (void)output_type; (void)output_structure; (void)propq;
    /* Simulate allocation/creation failure -> returns NULL */
    return NULL;
}

static volatile int g_sink;
int OSSL_ENCODER_to_bio(OSSL_ENCODER_CTX *ctx, BIO *out)
{
    /* Vulnerable library side: assume ctx is non-NULL and dereference it */
    (void)out;
    /* This will crash (NULL deref) when ctx == NULL, as in the reported bug */
    g_sink = ctx->dummy; /* Intentional NULL dereference to simulate library behavior */
    return 1;
}

int OSSL_ENCODER_CTX_set_cipher(OSSL_ENCODER_CTX *ctx, const char *name, const char *props)
{ (void)ctx; (void)name; (void)props; return 1; }
int OSSL_ENCODER_CTX_set_passphrase_ui(OSSL_ENCODER_CTX *ctx, const void *ui, void *data)
{ (void)ctx; (void)ui; (void)data; return 1; }
int OSSL_ENCODER_CTX_set_passphrase(OSSL_ENCODER_CTX *ctx, const unsigned char *pass, size_t len)
{ (void)ctx; (void)pass; (void)len; return 1; }
const char *EVP_CIPHER_get0_name(const EVP_CIPHER *ciph)
{ (void)ciph; return "AES-256-CBC"; }

/* BIO printf stub used by ec_main */
int BIO_printf(BIO *b, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    int r = vfprintf(b && b->fp ? b->fp : stdout, fmt, ap);
    va_end(ap);
    return r;
}

int main(void)
{
    /* Set up minimal state similar to the vulnerable block in apps/ec.c */
    EVP_PKEY *eckey = (EVP_PKEY *)malloc(sizeof(*eckey));
    if (!eckey) return 1;

    BIO out_bio = { stdout };
    BIO *out = &out_bio;

    int noout = 0;
    int param_out = 0;
    int pubin = 0;
    int pubout = 0;
    int private_key_expected = 1; /* matches assert(private) in original code */
    EVP_CIPHER *enc = NULL;       /* Keep NULL to skip cipher/passphrase block */

    if (!noout) {
        int selection;
        const char *output_type = "PEM";            /* outformat != ASN1 -> PEM */
        const char *output_structure = "type-specific";

        BIO_printf(out, "writing EC key\n");
        if (param_out) {
            selection = OSSL_KEYMGMT_SELECT_DOMAIN_PARAMETERS;
        } else if (pubin || pubout) {
            selection = OSSL_KEYMGMT_SELECT_DOMAIN_PARAMETERS | OSSL_KEYMGMT_SELECT_PUBLIC_KEY;
            output_structure = "SubjectPublicKeyInfo";
        } else {
            selection = OSSL_KEYMGMT_SELECT_ALL;
            assert(private_key_expected);
        }

        /* This will return NULL by design (simulating failure) */
        OSSL_ENCODER_CTX *ectx = OSSL_ENCODER_CTX_new_for_pkey(eckey, selection,
                                                               output_type, output_structure,
                                                               NULL);
        /* enc == NULL, so skip the block that would set cipher/passphrase */

        /* Vulnerable call: ectx is NULL, passed into OSSL_ENCODER_to_bio -> NULL deref */
        if (!OSSL_ENCODER_to_bio(ectx, out)) {
            BIO_printf(out, "unable to write EC key\n");
        }
    }

    free(eckey);
    return 0; /* We expect the program to crash before this due to NULL deref */
}
