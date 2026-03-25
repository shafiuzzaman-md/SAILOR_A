// Standalone C reproducer for NULL deref in app_http_get_asn1
// Compile: clang -fsanitize=address -g -O0 reproducer.c -o reproducer
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

// Minimal OpenSSL-like type stubs
typedef struct bio_st {
    int dummy;
} BIO;

typedef struct ssl_ctx_st {
    int dummy;
} SSL_CTX;

typedef struct asn1_item_st {
    int dummy;
} ASN1_ITEM;

typedef struct asn1_value_st {
    int dummy;
} ASN1_VALUE;

// CONF_VALUE and STACK_OF emulation
typedef struct conf_value_st {
    int dummy;
} CONF_VALUE;

#define STACK_OF(type) struct stack_st_##type
STACK_OF(CONF_VALUE) { int dummy; };

// Constants emulating OpenSSL error/library codes
#define ERR_LIB_HTTP  0x1234
#define ERR_R_PASSED_NULL_PARAMETER 1
#define ERR_R_PASSED_INVALID_ARGUMENT 2
#define OSSL_HTTP_DEFAULT_MAX_RESP_LEN 1048576UL

// APP_HTTP_TLS_INFO as used in apps/lib/apps.c
typedef struct {
    const char *server;
    const char *port;
    int use_proxy;
    long timeout;
    SSL_CTX *ssl_ctx;
} APP_HTTP_TLS_INFO;

// Function prototypes (stubs) to satisfy calls from app_http_get_asn1
int OSSL_HTTP_parse_url(const char *url, int *pssl, char **userinfo,
                        char **host, char **port, int *port_num,
                        char **path, char **query, char **frag);
const char *OSSL_HTTP_adapt_proxy(const char *proxy, const char *no_proxy,
                                  const char *server, int use_ssl);

typedef int (*OSSL_HTTP_bio_cb_t)(BIO *bio, void *arg, int keep_alive);
BIO *OSSL_HTTP_get(const char *url, const char *proxy, const char *no_proxy,
                   BIO *bio, BIO *rbio, OSSL_HTTP_bio_cb_t cb, void *arg,
                   int buf_size, const STACK_OF(CONF_VALUE) *headers,
                   const char *expected_content_type, int expect_asn1,
                   unsigned long max_resp_len, long timeout);

// Error handling stubs
void ERR_raise(int lib, int reason) {
    fprintf(stderr, "ERR_raise(lib=%d, reason=%d)\n", lib, reason);
}
void ERR_raise_data(int lib, int reason, const char *data) {
    fprintf(stderr, "ERR_raise_data(lib=%d, reason=%d, data=%s)\n", lib, reason, data ? data : "(null)");
}

// Memory helpers
void OPENSSL_free(void *p) { free(p); }

// BIO helpers
void BIO_free(BIO *b) { (void)b; /* nothing to do for this stub */ }

// TLS callback stub (not used but matches signature)
int app_http_tls_cb(BIO *bio, void *arg, int keep_alive) {
    (void)bio; (void)arg; (void)keep_alive; return 1;
}

// ASN.1 decode from BIO stub: intentionally dereferences BIO pointer
ASN1_VALUE *ASN1_item_d2i_bio(const ASN1_ITEM *it, BIO *in, ASN1_VALUE **val) {
    (void)it; (void)val;
    // This simulates internal use of BIO (e.g., BIO_read), which will
    // crash if 'in' is NULL due to NULL pointer dereference.
    // The following line intentionally dereferences 'in'.
    int x = in->dummy; // NULL deref here when 'in' is NULL
    (void)x;
    return NULL;
}

// The vulnerable function (adapted from apps/lib/apps.c)
ASN1_VALUE *app_http_get_asn1(const char *url, const char *proxy,
    const char *no_proxy, SSL_CTX *ssl_ctx,
    const STACK_OF(CONF_VALUE) *headers,
    long timeout, const char *expected_content_type,
    const ASN1_ITEM *it)
{
    APP_HTTP_TLS_INFO info;
    char *server;
    char *port;
    int use_ssl;
    BIO *mem;
    ASN1_VALUE *resp = NULL;

    if (url == NULL || it == NULL) {
        ERR_raise(ERR_LIB_HTTP, ERR_R_PASSED_NULL_PARAMETER);
        return NULL;
    }

    if (!OSSL_HTTP_parse_url(url, &use_ssl, NULL /* userinfo */, &server, &port,
            NULL /* port_num */, NULL, NULL, NULL))
        return NULL;
    if (use_ssl && ssl_ctx == NULL) {
        ERR_raise_data(ERR_LIB_HTTP, ERR_R_PASSED_NULL_PARAMETER,
            "missing SSL_CTX");
        goto end;
    }
    if (!use_ssl && ssl_ctx != NULL) {
        ERR_raise_data(ERR_LIB_HTTP, ERR_R_PASSED_INVALID_ARGUMENT,
            "SSL_CTX given but use_ssl == 0");
        goto end;
    }

    info.server = server;
    info.port = port;
    info.use_proxy = /* workaround for callback design flaw, see #17088 */
        OSSL_HTTP_adapt_proxy(proxy, no_proxy, server, use_ssl) != NULL;
    info.timeout = timeout;
    info.ssl_ctx = ssl_ctx;
    mem = OSSL_HTTP_get(url, proxy, no_proxy, NULL /* bio */, NULL /* rbio */,
        app_http_tls_cb, &info, 0 /* buf_size */, headers,
        expected_content_type, 1 /* expect_asn1 */,
        OSSL_HTTP_DEFAULT_MAX_RESP_LEN, timeout);
    // Vulnerable call: no NULL check on 'mem'
    resp = ASN1_item_d2i_bio(it, mem, NULL);
    BIO_free(mem);

end:
    OPENSSL_free(server);
    OPENSSL_free(port);
    return resp;
}

// --- Stub implementations ---
int OSSL_HTTP_parse_url(const char *url, int *pssl, char **userinfo,
                        char **host, char **port, int *port_num,
                        char **path, char **query, char **frag)
{
    (void)userinfo; (void)port_num; (void)path; (void)query; (void)frag;
    if (!url || !pssl || !host || !port) return 0;
    // Always pretend it's plain HTTP (no SSL), and allocate server/port
    *pssl = 0;
    const char *fake_host = "example.com";
    const char *fake_port = "80";
    *host = strdup(fake_host);
    *port = strdup(fake_port);
    if (*host == NULL || *port == NULL) {
        free(*host);
        free(*port);
        return 0;
    }
    return 1;
}

const char *OSSL_HTTP_adapt_proxy(const char *proxy, const char *no_proxy,
                                  const char *server, int use_ssl)
{
    (void)proxy; (void)no_proxy; (void)server; (void)use_ssl;
    // Return NULL meaning no proxy in use
    return NULL;
}

BIO *OSSL_HTTP_get(const char *url, const char *proxy, const char *no_proxy,
                   BIO *bio, BIO *rbio, OSSL_HTTP_bio_cb_t cb, void *arg,
                   int buf_size, const STACK_OF(CONF_VALUE) *headers,
                   const char *expected_content_type, int expect_asn1,
                   unsigned long max_resp_len, long timeout)
{
    (void)url; (void)proxy; (void)no_proxy; (void)bio; (void)rbio;
    (void)cb; (void)arg; (void)buf_size; (void)headers;
    (void)expected_content_type; (void)expect_asn1; (void)max_resp_len; (void)timeout;
    // Simulate a network/HTTP failure: return NULL
    return NULL;
}

int main(void) {
    // Prepare minimal valid inputs to pass initial checks
    const char *url = "http://example.com/test";
    const char *proxy = NULL;
    const char *no_proxy = NULL;
    SSL_CTX *ssl_ctx = NULL; // No SSL, consistent with parse_url stub
    const STACK_OF(CONF_VALUE) *headers = NULL;
    long timeout = 1;
    const char *expected_content_type = "application/octet-stream";
    static const ASN1_ITEM dummy_it; // Non-NULL ASN1 item pointer

    fprintf(stderr, "Calling app_http_get_asn1; this should crash due to NULL BIO...\n");
    // This will crash inside ASN1_item_d2i_bio because OSSL_HTTP_get returns NULL
    ASN1_VALUE *v = app_http_get_asn1(url, proxy, no_proxy, ssl_ctx,
                                      headers, timeout,
                                      expected_content_type, &dummy_it);
    // Unreachable if crash occurs as intended
    (void)v;
    fprintf(stderr, "Unexpectedly survived without crash.\n");
    return 0;
}
