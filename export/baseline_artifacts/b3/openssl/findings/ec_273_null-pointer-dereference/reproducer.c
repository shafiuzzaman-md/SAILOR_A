#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Minimal stub type definitions to avoid needing OpenSSL headers */
typedef struct evp_pkey_st { int dummy; } EVP_PKEY;
typedef struct evp_cipher_st { const char *name; } EVP_CIPHER;
typedef struct ossl_encoder_ctx_st { int dummy; } OSSL_ENCODER_CTX;

/* Stubbed OpenSSL-like APIs used by apps/ec.c */
OSSL_ENCODER_CTX *OSSL_ENCODER_CTX_new_for_pkey(EVP_PKEY *pkey, int selection,
                                                const char *output_type,
                                                const char *output_structure,
                                                const char *propq)
{
    (void)pkey; (void)selection; (void)output_type; (void)output_structure; (void)propq;
    /* Simulate failure (e.g., OOM or unsupported selection) returning NULL */
    return NULL;
}

/* Simulate EVP_CIPHER_get0_name() */
const char *EVP_CIPHER_get0_name(const EVP_CIPHER *c)
{
    if (c && c->name) return c->name;
    return "(null-cipher)";
}

/* This mimics the library's expectation that ctx is non-NULL.
 * We deliberately dereference ctx to demonstrate the NULL-deref that
 * apps/ec.c can trigger when it fails to check the return from
 * OSSL_ENCODER_CTX_new_for_pkey(). */
int OSSL_ENCODER_CTX_set_cipher(OSSL_ENCODER_CTX *ctx, const char *cipher_name, const char *propq)
{
    (void)cipher_name; (void)propq;
    /* Intentional dereference to crash if ctx == NULL */
    volatile unsigned char crash = *((unsigned char *)ctx);
    (void)crash;
    return 1;
}

int main(void)
{
    /* Values analogous to those in apps/ec.c */
    EVP_PKEY *eckey = NULL; /* Not needed for the reproducer */
    int selection = 0xFFFF; /* Placeholder for OSSL_KEYMGMT_SELECT_ALL */
    const char *output_type = "PEM";
    const char *output_structure = "type-specific";

    /* Make sure enc != NULL so the vulnerable path is taken */
    EVP_CIPHER *enc = (EVP_CIPHER *)malloc(sizeof(*enc));
    if (!enc) {
        fprintf(stderr, "malloc failed\n");
        return 1;
    }
    enc->name = "AES-256-CBC";

    /* This returns NULL to simulate the error condition apps/ec.c ignores */
    OSSL_ENCODER_CTX *ectx = OSSL_ENCODER_CTX_new_for_pkey(
        eckey, selection, output_type, output_structure, NULL);

    /* This mirrors line 273 in apps/ec.c: enc != NULL triggers the call.
     * Because ectx is NULL, our stub will dereference NULL and crash. */
    if (enc != NULL) {
        (void)OSSL_ENCODER_CTX_set_cipher(ectx, EVP_CIPHER_get0_name(enc), NULL);
    }

    /* Not reached due to crash */
    free(enc);
    return 0;
}
