#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Minimal stubs and types to simulate the OpenSSL app environment */

typedef struct OSSL_PARAM_st { int dummy; } OSSL_PARAM;

typedef struct EVP_MAC_st { int dummy; } EVP_MAC;

typedef struct EVP_MAC_CTX_st { int dummy; } EVP_MAC_CTX;

/* Simple stack of C strings to simulate sk_OPENSSL_STRING */
typedef struct {
    char **data;
    int num;
    int cap;
} OPENSSL_STRING_STACK;

static OPENSSL_STRING_STACK *sk_OPENSSL_STRING_new_null(void) {
    OPENSSL_STRING_STACK *s = (OPENSSL_STRING_STACK *)calloc(1, sizeof(*s));
    return s;
}

static int sk_OPENSSL_STRING_push(OPENSSL_STRING_STACK *s, char *str) {
    if (s == NULL) return 0;
    if (s->num == s->cap) {
        int ncap = s->cap ? s->cap * 2 : 4;
        char **ndata = (char **)realloc(s->data, ncap * sizeof(*ndata));
        if (!ndata) return 0;
        s->data = ndata;
        s->cap = ncap;
    }
    s->data[s->num++] = str;
    return 1;
}

static int sk_OPENSSL_STRING_num(const OPENSSL_STRING_STACK *s) { return s ? s->num : 0; }
static char *sk_OPENSSL_STRING_value(const OPENSSL_STRING_STACK *s, int i) { return s->data[i]; }

/* Stubs for OpenSSL memory and BIO helpers */
static void *OPENSSL_malloc(size_t n) { return malloc(n); }
static void OPENSSL_free(void *p) { free(p); }

#define BUFSIZE 4096
static void *app_malloc(size_t n, const char *what) {
    void *p = malloc(n);
    if (!p) {
        fprintf(stderr, "Failed to allocate %s\n", what ? what : "memory");
        exit(1);
    }
    return p;
}

/* Minimal BIO printf to stderr */
#define BIO void
static BIO *bio_err = (BIO *)0x1; /* dummy non-NULL */
static int BIO_printf(BIO *b, const char *fmt, ...) {
    (void)b;
    va_list ap;
    va_start(ap, fmt);
    int r = vfprintf(stderr, fmt, ap);
    va_end(ap);
    return r;
}

static void ERR_print_errors(BIO *b) { (void)b; }

/* Option parsing stubs */

enum {
    OPT_EOF = -1,
    OPT_HELP = 0,
    OPT_BIN,
    OPT_IN,
    OPT_OUT,
    OPT_MACOPT,
    OPT_CIPHER,
    OPT_DIGEST,
    OPT_PROV_CASES = 1000
};

static int g_argc;
static char **g_argv;
static int g_idx;
static char *g_opt_arg;

static const char *opt_init(int argc, char **argv, void *unused) {
    (void)unused;
    g_argc = argc;
    g_argv = argv;
    g_idx = 1; /* skip program name */
    g_opt_arg = NULL;
    return argv[0];
}

static int opt_next(void) {
    if (g_idx >= g_argc) return OPT_EOF;
    char *a = g_argv[g_idx++];
    if (a[0] != '-') {
        /* Non-option: put back and finish options */
        g_idx--; 
        return OPT_EOF;
    }
    if (strcmp(a, "-digest") == 0) {
        if (g_idx >= g_argc) return OPT_EOF;
        g_opt_arg = g_argv[g_idx++];
        return OPT_DIGEST;
    }
    /* Ignore/unknown options -> trigger opthelp in caller if needed */
    return 9999; /* default case */
}

static char *opt_arg(void) { return g_opt_arg; }

static int opt_check_rest_arg(const char *what) {
    (void)what;
    return g_idx < g_argc; /* require at least one arg left (MAC name) */
}

static char **opt_rest(void) { return &g_argv[g_idx]; }

static void opt_help(void *unused) { (void)unused; }

/* Stubs for MAC API */
static EVP_MAC *EVP_MAC_fetch(void *libctx, const char *name, const char *propq) {
    (void)libctx; (void)name; (void)propq;
    return (EVP_MAC *)OPENSSL_malloc(sizeof(EVP_MAC));
}

static EVP_MAC_CTX *EVP_MAC_CTX_new(EVP_MAC *mac) {
    if (!mac) return NULL;
    return (EVP_MAC_CTX *)OPENSSL_malloc(sizeof(EVP_MAC_CTX));
}

static void *EVP_MAC_settable_ctx_params(EVP_MAC *mac) { (void)mac; return (void*)0x1; }
static int EVP_MAC_CTX_set_params(EVP_MAC_CTX *ctx, const OSSL_PARAM *p) { (void)ctx; (void)p; return 1; }

static void app_params_free(OSSL_PARAM *p) { if (p) OPENSSL_free(p); }

/* This function will iterate the 'opts' stack and read the strings, 
   thereby dereferencing any stale pointer left there (UAF trigger). */
static OSSL_PARAM *app_params_new_from_opts(OPENSSL_STRING_STACK *opts, void *unused) {
    (void)unused;
    if (!opts) return NULL;
    volatile size_t total = 0;
    for (int i = 0; i < sk_OPENSSL_STRING_num(opts); i++) {
        char *s = sk_OPENSSL_STRING_value(opts, i);
        /* Access freed memory if s points to previously freed block */
        if (s) {
            /* strlen causes reads over the whole string */
            size_t len = strlen(s);
            total += len;
            /* Also read first and last byte where possible */
            if (len > 0) {
                volatile char c1 = s[0];
                volatile char c2 = s[len - 1];
                (void)c1; (void)c2;
            }
        }
    }
    /* Return any non-NULL OSSL_PARAM pointer so caller continues. */
    OSSL_PARAM *p = (OSSL_PARAM *)OPENSSL_malloc(sizeof(OSSL_PARAM));
    return p;
}

/* Helper to simulate the real alloc_mac_algorithm_name behavior: 
   - Allocate a new string that is ALSO pushed into opts
   - Return the same pointer so the caller stores it in 'digest'
   This ensures freeing 'digest' leaves a stale pointer inside 'opts'. */
static char *alloc_mac_algorithm_name(OPENSSL_STRING_STACK **opts,
                                      const char *type, const char *val) {
    if (*opts == NULL) *opts = sk_OPENSSL_STRING_new_null();
    if (*opts == NULL) return NULL;
    /* Build a string like "type:val"; length chosen to vary sizes */
    size_t tlen = strlen(type), vlen = strlen(val);
    size_t len = tlen + 1 + vlen + 1;
    char *s = (char *)OPENSSL_malloc(len);
    if (!s) return NULL;
    snprintf(s, len, "%s:%s", type, val);
    if (!sk_OPENSSL_STRING_push(*opts, s)) {
        OPENSSL_free(s);
        return NULL;
    }
    return s;
}

/* Dummy getters used by the original code */
static void *app_get0_libctx(void) { return NULL; }
static const char *app_get0_propq(void) { return NULL; }

/* This is a minimized version of mac_main focused on the vulnerable path */
static int mac_main(int argc, char **argv) {
    const char *prog = NULL;
    int o;
    unsigned char *buf = NULL;
    const char *infile = NULL;
    int out_bin = 0;
    int inform = 0; /* FORMAT_BINARY */
    char *digest = NULL, *cipher = NULL;
    OSSL_PARAM *params = NULL;
    EVP_MAC *mac = NULL;
    EVP_MAC_CTX *ctx = NULL;
    OPENSSL_STRING_STACK *opts = NULL;
    char **rest_argv = NULL;
    (void)infile; (void)out_bin; (void)inform; (void)cipher;

    prog = opt_init(argc, argv, NULL);
    buf = (unsigned char *)app_malloc(BUFSIZE, "I/O buffer");

    while ((o = opt_next()) != OPT_EOF) {
        switch (o) {
        default:
        opthelp:
            BIO_printf(bio_err, "%s: Use -help for summary.\n", prog);
            goto err;
        case OPT_DIGEST:
            OPENSSL_free(digest);
            digest = alloc_mac_algorithm_name(&opts, "digest", opt_arg());
            if (digest == NULL)
                goto opthelp;
            break;
        }
    }

    /* One argument, the MAC name. */
    if (!opt_check_rest_arg("MAC name"))
        goto opthelp;
    rest_argv = opt_rest();

    mac = EVP_MAC_fetch(app_get0_libctx(), rest_argv[0], app_get0_propq());
    if (mac == NULL) {
        BIO_printf(bio_err, "Invalid MAC name %s\n", rest_argv[0]);
        goto opthelp;
    }

    ctx = EVP_MAC_CTX_new(mac);
    if (ctx == NULL)
        goto err;

    if (opts != NULL) {
        int ok = 1;
        params = app_params_new_from_opts(opts, EVP_MAC_settable_ctx_params(mac));
        if (params == NULL)
            goto err;
        if (!EVP_MAC_CTX_set_params(ctx, params)) {
            BIO_printf(bio_err, "MAC parameter error\n");
            ERR_print_errors(bio_err);
            ok = 0;
        }
        app_params_free(params);
        (void)ok;
    }

err:
    /* Intentionally do not clean up 'opts' stack entries to avoid double-free.
       The bug has already been triggered in app_params_new_from_opts. */
    if (ctx) OPENSSL_free(ctx);
    if (mac) OPENSSL_free(mac);
    if (buf) free(buf);
    if (digest) OPENSSL_free(digest);
    if (cipher) OPENSSL_free(cipher);
    if (opts) {
        /* Free the container only; leave strings to avoid double free */
        free(opts->data);
        free(opts);
    }
    return 0;
}

int main(void) {
    /* Craft arguments:
       - Two -digest options. The first allocated string will be freed when
         the second -digest is parsed but its pointer remains in 'opts'.
       - A final MAC algorithm name (e.g., HMAC) as the required positional arg. */
    char *argv[] = {
        (char *)"reproducer",
        (char *)"-digest", (char *)"AAAAAA", /* small allocation */
        (char *)"-digest", (char *)"BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB", /* much larger to avoid reuse */
        (char *)"HMAC",
        NULL
    };
    int argc = 5;

    /* Running mac_main will reach app_params_new_from_opts which iterates
       over 'opts' and dereferences the stale pointer, triggering UAF. */
    return mac_main(argc, argv);
}
