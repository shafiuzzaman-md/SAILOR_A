#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* Minimal stand-ins for OpenSSL types/APIs used by dgst.c */
typedef struct evp_md_st {
    int is_xof; /* non-zero if it is an XOF */
} EVP_MD;

/* Vulnerable helper: dereferences md without NULL check, like EVP_MD_xof(md) */
int EVP_MD_xof(const EVP_MD *md) {
    /* This will crash if md == NULL (simulating the OpenSSL bug site) */
    return md->is_xof;
}

/* Dummy SHA256 object to mimic non-XOF digest when md is initialized */
static EVP_MD DUMMY_SHA256 = { 0 };

static int strcasestr_like(const char *hay, const char *need) {
    size_t n = strlen(need);
    for (; *hay; hay++) {
        if (strncasecmp(hay, need, n) == 0)
            return 1;
    }
    return 0;
}

/* A very small facsimile of apps/dgst.c:dgst_main that exercises the bug path */
int dgst_main(int argc, char **argv)
{
    EVP_MD *md = NULL;          /* stays NULL for oneshot (Ed25519/Ed448) */
    int oneshot_sign = 0;       /* set to 1 when using Ed25519 signing key */
    void *sigkey = NULL;        /* non-NULL when -sign is provided */
    int xoflen = 0;             /* set via -xoflen */

    /* Parse minimal set of args: -sign <keyfile>, -xoflen <n> */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-sign") == 0 && i + 1 < argc) {
            const char *keyfile = argv[++i];
            sigkey = (void*)0x1; /* pretend we loaded a key */
            /* Simulate Ed25519/Ed448 oneshot behavior based on filename */
            if (strcasestr_like(keyfile, "ed25519") || strcasestr_like(keyfile, "ed448"))
                oneshot_sign = 1; /* oneshot sign path (no md initialization) */
        } else if (strcmp(argv[i], "-xoflen") == 0 && i + 1 < argc) {
            xoflen = atoi(argv[++i]);
        }
    }

    /* In the real code, if not oneshot_sign the digest might be initialized here. */
    if (!oneshot_sign) {
        /* md would be set to a default like SHA-256 for non-oneshot algorithms */
        md = &DUMMY_SHA256; /* non-XOF digest */
    }

    /* The vulnerable site: with oneshot_sign == 1, md remains NULL, but -xoflen > 0
     * leads to EVP_MD_xof(md), which dereferences md and crashes. */
    if (xoflen > 0) {
        /* This call will NULL-deref when md == NULL (expected for Ed25519/Ed448 + -sign) */
        if (!EVP_MD_xof(md)) {
            /* In the real code this prints an error, but we won't reach here on crash */
            fprintf(stderr, "Length can only be specified for XOF\n");
        }
    }

    return 0;
}

int main(void)
{
    /* Craft argv to match the concrete crash path:
     * -sign with an Ed25519 key (oneshot_sign == 1), and -xoflen > 0.
     * md remains NULL and EVP_MD_xof(md) dereferences NULL.
     */
    char *argv[] = {
        (char*)"reproducer",
        (char*)"-sign", (char*)"test-ed25519-key.pem",
        (char*)"-xoflen", (char*)"32",
        NULL
    };
    int argc = 5;

    /* Call the vulnerable main */
    return dgst_main(argc, argv);
}
