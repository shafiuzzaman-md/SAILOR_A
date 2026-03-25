#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

/* Minimal stand-ins for the OpenSSL types/APIs used in apps/kdf.c */

typedef struct stack_st_OPENSSL_STRING {
    size_t num;
    size_t cap;
    char **data;
} STACK_OF_OPENSSL_STRING;

static STACK_OF_OPENSSL_STRING *sk_OPENSSL_STRING_new_null(void) {
    STACK_OF_OPENSSL_STRING *sk = (STACK_OF_OPENSSL_STRING *)calloc(1, sizeof(*sk));
    return sk;
}

static int sk_OPENSSL_STRING_push(STACK_OF_OPENSSL_STRING *sk, char *s) {
    if (sk == NULL) return 0;
    if (sk->num == sk->cap) {
        size_t ncap = sk->cap ? sk->cap * 2 : 4;
        char **nd = (char **)realloc(sk->data, ncap * sizeof(*nd));
        if (nd == NULL) return 0;
        sk->data = nd;
        sk->cap = ncap;
    }
    sk->data[sk->num++] = s;
    return 1;
}

static void sk_OPENSSL_STRING_free(STACK_OF_OPENSSL_STRING *sk) {
    if (!sk) return;
    /* Intentionally do NOT free the elements here to simulate app lifetime. */
    free(sk->data);
    free(sk);
}

/* OPENSSL_free is just free() for our purposes */
static void OPENSSL_free(void *p) {
    free(p);
}

/*
 * Mimics alloc_kdf_algorithm_name(&opts, name, value) from apps/kdf.c.
 * It allocates a single string that is also pushed into opts and returned
 * to the caller (double ownership). This mirrors the problematic ownership
 * that leads to the use-after-free when the caller frees the previous
 * "cipher" value but doesn't remove it from opts.
 */
static char *alloc_kdf_algorithm_name(STACK_OF_OPENSSL_STRING **popts,
                                      const char *name, const char *value) {
    size_t nlen = strlen(name);
    size_t vlen = strlen(value);
    size_t len = nlen + 1 /* ':' */ + vlen + 1 /* NUL */;
    char *buf = (char *)malloc(len);
    if (buf == NULL) return NULL;
    snprintf(buf, len, "%s:%s", name, value);

    if (*popts == NULL)
        *popts = sk_OPENSSL_STRING_new_null();
    if (*popts == NULL) {
        free(buf);
        return NULL;
    }
    if (!sk_OPENSSL_STRING_push(*popts, buf)) {
        free(buf);
        return NULL;
    }
    return buf; /* Same pointer is also stored in opts */
}

/*
 * Stand-in for app_params_new_from_opts(opts, ...).
 * It iterates over opts, dereferences each entry.
 * If an entry points to freed memory (due to the bug), ASan will detect it.
 */
static void app_params_new_from_opts(STACK_OF_OPENSSL_STRING *opts) {
    if (opts == NULL) return;
    volatile unsigned sum = 0;
    for (size_t i = 0; i < opts->num; i++) {
        char *s = opts->data[i];
        if (s) {
            /* Touch the memory to trigger UAF under ASan if freed. */
            sum += (unsigned)(unsigned char)s[0];
        }
    }
    /* Prevent optimizing away */
    fprintf(stderr, "sum=%u\n", sum);
}

int main(void) {
    STACK_OF_OPENSSL_STRING *opts = NULL;
    char *cipher = NULL;

    /* First -cipher option: allocate and store in both 'cipher' and 'opts' */
    cipher = alloc_kdf_algorithm_name(&opts, "cipher", "AES-128-CBC");
    if (!cipher) {
        fprintf(stderr, "alloc_kdf_algorithm_name first failed\n");
        return 1;
    }

    /* Second -cipher option: the buggy path frees the previous 'cipher' */
    /* apps/kdf.c line 116 equivalent */
    OPENSSL_free(cipher); /* BUG: the same pointer still lives inside 'opts' */

    /* Replace with a new cipher value (like processing a repeated -cipher) */
    cipher = alloc_kdf_algorithm_name(&opts, "cipher", "AES-256-CBC");
    if (!cipher) {
        fprintf(stderr, "alloc_kdf_algorithm_name second failed\n");
        return 1;
    }

    /* Later, the app iterates over 'opts' and dereferences entries */
    /* This will read the freed pointer from the first -cipher and trigger UAF */
    app_params_new_from_opts(opts);

    /* Cleanup (won't be reached if ASan aborts on UAF) */
    sk_OPENSSL_STRING_free(opts);
    OPENSSL_free(cipher);
    return 0;
}
