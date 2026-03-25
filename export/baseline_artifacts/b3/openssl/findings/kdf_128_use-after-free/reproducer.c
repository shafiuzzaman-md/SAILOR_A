#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Minimal stand-ins for OpenSSL string stack and helpers */
typedef struct {
    size_t n;
    size_t cap;
    char **data;
} SK_OPENSSL_STRING;

static SK_OPENSSL_STRING *sk_OPENSSL_STRING_new_null(void) {
    SK_OPENSSL_STRING *st = (SK_OPENSSL_STRING *)calloc(1, sizeof(*st));
    return st;
}

static int sk_OPENSSL_STRING_push(SK_OPENSSL_STRING *st, char *s) {
    if (st == NULL)
        return 0;
    if (st->n == st->cap) {
        size_t newcap = st->cap ? st->cap * 2 : 4;
        char **nd = (char **)realloc(st->data, newcap * sizeof(char *));
        if (nd == NULL)
            return 0;
        st->data = nd;
        st->cap = newcap;
    }
    st->data[st->n++] = s;
    return 1;
}

/* OPENSSL_free equivalent */
static void OPENSSL_free(void *p) { free(p); }

/* Minimal OSSL_PARAM stand-in */
typedef struct {
    int dummy;
} OSSL_PARAM;

/* This mimics the behavior of alloc_kdf_algorithm_name(&opts, "mac", arg):
 * - allocate a string "type:arg"
 * - push the pointer to opts (without duplicating it)
 * - return the same pointer so that freeing it later leaves a dangling entry in opts
 */
static char *alloc_kdf_algorithm_name(SK_OPENSSL_STRING **popts, const char *type, const char *alg) {
    size_t tl = strlen(type);
    size_t al = strlen(alg);
    size_t len = tl + 1 + al + 1; /* type ':' alg '\0' */
    char *s = (char *)malloc(len);
    if (s == NULL)
        return NULL;
    memcpy(s, type, tl);
    s[tl] = ':';
    memcpy(s + tl + 1, alg, al);
    s[len - 1] = '\0';

    if (*popts == NULL)
        *popts = sk_OPENSSL_STRING_new_null();
    if (*popts == NULL) {
        free(s);
        return NULL;
    }
    if (!sk_OPENSSL_STRING_push(*popts, s)) {
        free(s);
        return NULL;
    }
    return s;
}

/* This mimics app_params_new_from_opts(opts, ...): iterate through opts and read strings. */
static OSSL_PARAM *app_params_new_from_opts(SK_OPENSSL_STRING *opts, void *ignored) {
    if (opts == NULL)
        return NULL;
    volatile size_t total = 0; /* volatile to prevent optimization */
    for (size_t i = 0; i < opts->n; i++) {
        char *p = opts->data[i];
        /* UAF will trigger here if p points to freed memory. */
        size_t len = strlen(p);
        total += len;
    }
    OSSL_PARAM *ret = (OSSL_PARAM *)malloc(sizeof(OSSL_PARAM));
    if (ret)
        ret->dummy = (int)total;
    return ret;
}

/* Reimplementation of the vulnerable path in kdf_main focusing on -mac handling. */
static int kdf_main_repro(void) {
    SK_OPENSSL_STRING *opts = NULL;
    char *mac = NULL;

    /* First -mac option: allocate a large string so its freed chunk is unlikely to be reused. */
    size_t big = 65536; /* 64 KiB */
    char *bigalg = (char *)malloc(big + 1);
    if (!bigalg) {
        fprintf(stderr, "alloc failed\n");
        return 1;
    }
    memset(bigalg, 'A', big);
    bigalg[big] = '\0';

    mac = alloc_kdf_algorithm_name(&opts, "mac", bigalg);
    if (mac == NULL) {
        fprintf(stderr, "alloc_kdf_algorithm_name (first) failed\n");
        return 1;
    }

    /* Second -mac option: frees previous 'mac' but does not remove it from opts. */
    OPENSSL_free(mac); /* Dangling entry remains in opts */

    const char *smallalg = "B"; /* small so allocator likely doesn't reuse the big freed chunk */
    mac = alloc_kdf_algorithm_name(&opts, "mac", smallalg);
    if (mac == NULL) {
        fprintf(stderr, "alloc_kdf_algorithm_name (second) failed\n");
        return 1;
    }

    /* Later, app_params_new_from_opts reads entries in opts, including the freed first one -> UAF */
    OSSL_PARAM *params = app_params_new_from_opts(opts, NULL);
    if (params)
        free(params);

    /* Cleanup. Avoid freeing entries in opts to prevent double-free of the already-freed one. */
    free(bigalg);
    free(opts->data); /* free container storage only */
    free(opts);

    return 0;
}

int main(void) {
    return kdf_main_repro();
}
