#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Minimal stand-ins for OpenSSL types/macros used by apps/lib/apps.c */

typedef struct ssl_ctx_st SSL_CTX; /* unused in this reproducer */

typedef struct asn1_item_st {
    int dummy;
} ASN1_ITEM;

typedef struct asn1_value_st {
    int dummy;
} ASN1_VALUE;

/* Implement STACK_OF(CONF_VALUE) compatible placeholder */
#define STACK_OF(type) struct stack_st_##type

typedef struct conf_value_st { int dummy; } CONF_VALUE;
struct stack_st_CONF_VALUE { int dummy; };

/* Minimal BIO type and functions */
typedef struct bio_st {
    unsigned char *data;
    size_t len;
} BIO;

/* Placeholder for info struct used in apps */
typedef struct app_http_tls_info_st {
    const char *server;
    const char *port;
    int use_proxy;
    long timeout;
    SSL_CTX *ssl_ctx;
} APP_HTTP_TLS_INFO;

/* Constants expected by code */
#define OSSL_HTTP_DEFAULT_MAX_RESP_LEN 1048576L

/* Stubs to mimic the OpenSSL API surface used by app_http_post_asn1 */

static void OPENSSL_free(void *p) { free(p); }

static char *OSSL_HTTP_adapt_proxy(const char *proxy, const char *no_proxy,
                                   const char *server, int use_ssl)
{
    /* For simplicity, always indicate no proxy adaptation result */
    (void)proxy; (void)no_proxy; (void)server; (void)use_ssl;
    return NULL; /* returning NULL is fine; caller only checks != NULL */
}

/* TLS callback placeholder */
static int app_http_tls_cb(void *rctx, void *arg)
{
    (void)rctx; (void)arg;
    return 1;
}

/* BIO helpers */
static BIO *BIO_new_mem_buf(const void *data, size_t len)
{
    BIO *b = (BIO *)malloc(sizeof(BIO));
    if (b == NULL) return NULL;
    b->data = (unsigned char *)malloc(len ? len : 1);
    if (b->data == NULL) { free(b); return NULL; }
    if (len)
        memcpy(b->data, data, len);
    b->len = len;
    return b;
}

static void BIO_free(BIO *b)
{
    if (b) {
        free(b->data);
        free(b);
    }
}

static int BIO_read(BIO *b, void *out, int len)
{
    /* Intentional dereference of b to simulate BIO access; if b == NULL, this
       will crash with a NULL pointer dereference, matching the real bug. */
    if (b == NULL) {
        /* Force a crash in a controlled way */
        volatile int *crash = NULL;
        *crash = 42; /* NULL deref */
        return -1; /* not reached */
    }
    if ((size_t)len > b->len) len = (int)b->len;
    memcpy(out, b->data, (size_t)len);
    return len;
}

/* The encoding function used to build the request body */
static BIO *ASN1_item_i2d_mem_bio(const ASN1_ITEM *it, ASN1_VALUE *val)
{
    (void)it; (void)val;
    const char payload[] = "dummy-asn1-request";
    return BIO_new_mem_buf(payload, sizeof(payload));
}

/* The decoding function that will attempt to read from the provided BIO */
static ASN1_VALUE *ASN1_item_d2i_bio(const ASN1_ITEM *it, BIO *in, void *ctx)
{
    (void)it; (void)ctx;
    unsigned char tmp[1];
    /* This will dereference 'in' (NULL in the buggy path) via BIO_read */
    (void)BIO_read(in, tmp, 1);
    return NULL;
}

/* Simulated HTTP transfer: return NULL to emulate a failed transfer */
static BIO *OSSL_HTTP_transfer(BIO *rctx,
                               const char *host, const char *port,
                               const char *path, int use_ssl,
                               const char *proxy, const char *no_proxy,
                               BIO *bio, BIO *rbio,
                               int (*tls_cb)(void *rctx, void *arg), void *cb_arg,
                               size_t buf_size, const STACK_OF(CONF_VALUE) *headers,
                               const char *content_type, BIO *req_mem,
                               const char *expected_content_type, int expect_asn1,
                               long max_resp_len, long timeout, int keep_alive)
{
    (void)rctx; (void)host; (void)port; (void)path; (void)use_ssl;
    (void)proxy; (void)no_proxy; (void)bio; (void)rbio; (void)tls_cb; (void)cb_arg;
    (void)buf_size; (void)headers; (void)content_type; (void)req_mem;
    (void)expected_content_type; (void)expect_asn1; (void)max_resp_len;
    (void)timeout; (void)keep_alive;
    /* Emulate failure -> NULL response BIO */
    return NULL;
}

/* The vulnerable function, adapted from apps/lib/apps.c */
static ASN1_VALUE *app_http_post_asn1(const char *host, const char *port,
                                      const char *path, const char *proxy,
                                      const char *no_proxy, SSL_CTX *ssl_ctx,
                                      const STACK_OF(CONF_VALUE) *headers,
                                      const char *content_type,
                                      ASN1_VALUE *req, const ASN1_ITEM *req_it,
                                      const char *expected_content_type,
                                      long timeout, const ASN1_ITEM *rsp_it)
{
    int use_ssl = ssl_ctx != NULL;
    APP_HTTP_TLS_INFO info;
    BIO *rsp, *req_mem = ASN1_item_i2d_mem_bio(req_it, req);
    ASN1_VALUE *res;

    if (req_mem == NULL)
        return NULL;

    info.server = host;
    info.port = port;
    info.use_proxy = OSSL_HTTP_adapt_proxy(proxy, no_proxy, host, use_ssl) != NULL;
    info.timeout = timeout;
    info.ssl_ctx = ssl_ctx;
    rsp = OSSL_HTTP_transfer(NULL, host, port, path, use_ssl,
                             proxy, no_proxy, NULL /* bio */, NULL /* rbio */,
                             app_http_tls_cb, &info,
                             0 /* buf_size */, headers, content_type, req_mem,
                             expected_content_type, 1 /* expect_asn1 */,
                             OSSL_HTTP_DEFAULT_MAX_RESP_LEN, timeout,
                             0 /* keep_alive */);
    BIO_free(req_mem);
    /* Bug: rsp may be NULL on failure, but is passed unchecked */
    res = ASN1_item_d2i_bio(rsp_it, rsp, NULL);
    BIO_free(rsp);
    return res;
}

int main(void)
{
    /* Prepare minimal, non-NULL request and item so req_mem is produced */
    ASN1_ITEM req_it = {0};
    ASN1_ITEM rsp_it = {0};
    ASN1_VALUE req = {0};

    /* This call will cause OSSL_HTTP_transfer to return NULL, leading to
       ASN1_item_d2i_bio(NULL, ...) and a NULL dereference in BIO_read */
    (void)app_http_post_asn1("example.invalid", "443", "/test", NULL, NULL,
                             NULL /* ssl_ctx */, NULL /* headers */,
                             "application/octet-stream",
                             &req, &req_it,
                             "application/octet-stream",
                             5L, &rsp_it);

    /* If we somehow did not crash (shouldn't happen), indicate outcome */
    puts("If you see this, the NULL deref did not trigger as expected.");
    return 0;
}
