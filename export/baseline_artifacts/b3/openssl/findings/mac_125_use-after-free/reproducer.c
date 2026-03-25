#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Minimal stand-ins for OpenSSL-like APIs and types used in the snippet */
typedef struct OSSL_PARAM_st { int dummy; } OSSL_PARAM;

static void OPENSSL_free(void *p) { free(p); }

/* A minimal stack of C strings to emulate STACK_OF(OPENSSL_STRING) */
typedef struct OPENSSL_STRING_STACK {
    char **items;
    int num;
    int cap;
} OPENSSL_STRING_STACK;

static OPENSSL_STRING_STACK *sk_OPENSSL_STRING_new_null(void)
{
    OPENSSL_STRING_STACK *sk = (OPENSSL_STRING_STACK *)calloc(1, sizeof(*sk));
    if (sk == NULL)
        return NULL;
    sk->cap = 4;
    sk->items = (char **)calloc((size_t)sk->cap, sizeof(char *));
    if (sk->items == NULL) {
        free(sk);
        return NULL;
    }
    return sk;
}

static int sk_OPENSSL_STRING_push(OPENSSL_STRING_STACK *sk, char *s)
{
    if (sk == NULL)
        return 0;
    if (sk->num == sk->cap) {
        int ncap = sk->cap * 2;
        char **nitems = (char **)realloc(sk->items, (size_t)ncap * sizeof(char *));
        if (nitems == NULL)
            return 0;
        sk->items = nitems;
        sk->cap = ncap;
    }
    sk->items[sk->num++] = s;
    return 1;
}

/* This mimics alloc_mac_algorithm_name(&opts, "cipher", value)
 * It allocates a new string and ALSO pushes the SAME pointer into opts,
 * just like the real helper does when building the opts stack.
 */
static char *alloc_mac_algorithm_name(OPENSSL_STRING_STACK **popts,
                                      const char *key, const char *value)
{
    size_t klen = strlen(key);
    size_t vlen = strlen(value);
    char *s = (char *)malloc(klen + 1 /* ':' */ + vlen + 1 /* NUL */);
    if (s == NULL)
        return NULL;
    memcpy(s, key, klen);
    s[klen] = ':';
    memcpy(s + klen + 1, value, vlen);
    s[klen + 1 + vlen] = '\0';

    if (*popts == NULL)
        *popts = sk_OPENSSL_STRING_new_null();
    if (*popts == NULL || !sk_OPENSSL_STRING_push(*popts, s)) {
        free(s);
        return NULL;
    }
    return s;
}

/* This mimics app_params_new_from_opts(opts, ...) and iterates the opts stack.
 * It dereferences each stored pointer (including stale/freed ones),
 * which will trigger ASan Use-After-Free when it encounters the first -cipher value.
 */
static OSSL_PARAM *app_params_new_from_opts(OPENSSL_STRING_STACK *opts, void *ignored)
{
    (void)ignored;
    if (opts == NULL)
        return NULL;
    for (int i = 0; i < opts->num; i++) {
        char *s = opts->items[i];
        /* UAF: if s points to freed memory, strlen/printf will read freed bytes */
        volatile size_t l = strlen(s); /* Intentional read of potentially freed memory */
        printf("opts[%d] len=%zu value=%.32s\n", i, l, s);
    }
    /* Return any non-NULL value to mimic success */
    static OSSL_PARAM dummy;
    return &dummy;
}

int main(void)
{
    OPENSSL_STRING_STACK *opts = NULL;
    char *cipher = NULL;

    /* Simulate first -cipher <val1> option */
    cipher = alloc_mac_algorithm_name(&opts, "cipher", "AES-128-CBC");
    if (cipher == NULL) {
        fprintf(stderr, "alloc_mac_algorithm_name #1 failed\n");
        return 1;
    }

    /* Simulate second -cipher <val2> option: previous 'cipher' is freed,
     * but its pointer is still present inside 'opts', causing a stale entry.
     */
    OPENSSL_free(cipher); /* This leaves a dangling pointer inside opts */
    cipher = alloc_mac_algorithm_name(&opts, "cipher", "AES-256-CBC");
    if (cipher == NULL) {
        fprintf(stderr, "alloc_mac_algorithm_name #2 failed\n");
        return 1;
    }

    /* Later, app_params_new_from_opts walks 'opts' and dereferences entries.
     * The first entry is a UAF because it points to the freed string.
     */
    (void)app_params_new_from_opts(opts, NULL);

    /* Clean up minimal allocations we still own (not strictly necessary for the reproducer). */
    /* Free the remaining live pointer(s) to avoid leaks after the demonstration. */
    for (int i = 0; i < opts->num; i++) {
        /* Avoid double-free of the first (already freed) element by checking pointer equality. */
        if (opts->items[i] != NULL && opts->items[i] != cipher) {
            /* It may already be freed; in a robust impl we'd track that. Here we only free the last 'cipher'. */
        }
    }
    OPENSSL_free(cipher);
    free(opts->items);
    free(opts);

    return 0;
}
