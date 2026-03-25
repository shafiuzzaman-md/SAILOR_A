#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>

/* Minimal stand-ins for OpenSSL types */
typedef struct evp_pkey_st {
    int dummy;
} EVP_PKEY;

typedef struct ossl_encoder_ctx_st {
    int num_encoders;
} OSSL_ENCODER_CTX;

typedef struct bio_st BIO;

/* Constants mimicking those used in apps/dsa.c */
#define FORMAT_ASN1 0
#define FORMAT_PEM  1
#define FORMAT_MSBLOB 2
#define FORMAT_PVK 3

#define OSSL_KEYMGMT_SELECT_PUBLIC_KEY      0x01
#define OSSL_KEYMGMT_SELECT_KEYPAIR         0x02
#define OSSL_KEYMGMT_SELECT_ALL_PARAMETERS  0x04

/* Stub implementations for the OpenSSL API used in the vulnerable path */
static OSSL_ENCODER_CTX *OSSL_ENCODER_CTX_new_for_pkey(EVP_PKEY *pkey,
                                                       int selection,
                                                       const char *output_type,
                                                       const char *output_structure,
                                                       void *propq)
{
    (void)pkey;
    (void)selection;
    (void)output_type;
    (void)output_structure;
    (void)propq;
    /* Simulate allocation failure by returning NULL */
    return NULL;
}

__attribute__((noinline))
static int OSSL_ENCODER_CTX_get_num_encoders(OSSL_ENCODER_CTX *ctx)
{
    /* Vulnerable behavior: dereference without NULL check */
    return ctx->num_encoders; /* This will crash when ctx == NULL */
}

static void OSSL_ENCODER_CTX_free(OSSL_ENCODER_CTX *ctx)
{
    (void)ctx;
}

static void BIO_free_all(BIO *b)
{
    (void)b;
}

static void EVP_PKEY_free(EVP_PKEY *pkey)
{
    free(pkey);
}

static int BIO_printf(BIO *b, const char *fmt, ...)
{
    (void)b; /* In apps/dsa.c this prints to bio_err; we just forward to stderr */
    va_list ap;
    va_start(ap, fmt);
    int r = vfprintf(stderr, fmt, ap);
    va_end(ap);
    return r;
}

/* Minimal reproduction of the vulnerable segment from apps/dsa.c */
static void trigger_dsa_vuln(void)
{
    int ret = 1;
    BIO *bio_err = NULL; /* Unused in this reproducer except for BIO_printf */
    EVP_PKEY *pkey = (EVP_PKEY*)0x1; /* Dummy non-NULL pkey */
    int outformat = FORMAT_PEM;      /* Choose PEM/ASN1 to set output_structure */
    const char *output_type = NULL;
    const char *output_structure = NULL;

    int pubout = 0, pubin = 0;
    int private_key = 1;
    int selection = 0;

    /* This mirrors the control flow leading to the vulnerable call */
    if (outformat == FORMAT_ASN1) {
        output_type = "DER";
    } else if (outformat == FORMAT_PEM) {
        output_type = "PEM";
    } else if (outformat == FORMAT_MSBLOB) {
        output_type = "MSBLOB";
    } else if (outformat == FORMAT_PVK) {
        if (pubin) {
            BIO_printf(bio_err, "PVK form impossible with public key input\n");
            goto end;
        }
        output_type = "PVK";
    } else {
        BIO_printf(bio_err, "bad output format specified for outfile\n");
        goto end;
    }

    if (outformat == FORMAT_ASN1 || outformat == FORMAT_PEM) {
        if (pubout || pubin)
            output_structure = "SubjectPublicKeyInfo";
        else
            output_structure = "type-specific";
    }

    if (pubout || pubin) {
        selection = OSSL_KEYMGMT_SELECT_PUBLIC_KEY;
    } else {
        assert(private_key);
        selection = (OSSL_KEYMGMT_SELECT_KEYPAIR | OSSL_KEYMGMT_SELECT_ALL_PARAMETERS);
    }

    /* Perform the encoding: stub returns NULL to simulate failure */
    OSSL_ENCODER_CTX *ectx = OSSL_ENCODER_CTX_new_for_pkey(pkey, selection,
                                                           output_type,
                                                           output_structure,
                                                           NULL);

    /* Vulnerable NULL dereference: ectx may be NULL, but it's used anyway */
    if (OSSL_ENCODER_CTX_get_num_encoders(ectx) == 0) {
        BIO_printf(bio_err, "%s format not supported\n", output_type);
        goto end;
    }

    ret = 0;

end:
    (void)ret;
    OSSL_ENCODER_CTX_free(ectx);
    BIO_free_all(bio_err);
    EVP_PKEY_free(pkey);
}

int main(void)
{
    /* Running this will dereference a NULL pointer inside
       OSSL_ENCODER_CTX_get_num_encoders, reproducing the bug. */
    trigger_dsa_vuln();
    return 0;
}
