#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Minimal stand-ins for the OpenSSL types/APIs used in the vulnerable path */

typedef struct x509_st {
    int dummy;
} X509;

#define FORMAT_UNDEF 0

/* Simple stack for X509 pointers */
typedef struct {
    size_t num;
    size_t cap;
    X509 **data;
} STACK_OF_X509;

static STACK_OF_X509 *sk_X509_new_null(void) {
    STACK_OF_X509 *sk = (STACK_OF_X509 *)calloc(1, sizeof(*sk));
    if (!sk) return NULL;
    sk->cap = 4;
    sk->data = (X509 **)calloc(sk->cap, sizeof(X509 *));
    if (!sk->data) {
        free(sk);
        return NULL;
    }
    return sk;
}

static int sk_X509_push(STACK_OF_X509 *sk, X509 *x) {
    if (!sk) return 0;
    if (sk->num == sk->cap) {
        size_t newcap = sk->cap * 2;
        X509 **ndata = (X509 **)realloc(sk->data, newcap * sizeof(X509 *));
        if (!ndata) return 0;
        sk->data = ndata;
        sk->cap = newcap;
    }
    sk->data[sk->num++] = x;
    return 1;
}

static void sk_X509_pop_free(STACK_OF_X509 *sk, void (*freefunc)(X509 *)) {
    if (!sk) return;
    for (size_t i = 0; i < sk->num; i++) {
        if (sk->data[i])
            freefunc(sk->data[i]);
    }
    free(sk->data);
    free(sk);
}

/* Stubbed cert loader returns a freshly malloc'd X509 to simulate a real cert */
static X509 *load_cert(const char *path, int fmt, const char *desc) {
    (void)path; (void)fmt; (void)desc;
    X509 *x = (X509 *)malloc(sizeof(X509));
    return x; /* Non-NULL to simulate successful load */
}

/* Free function matching OpenSSL's X509_free semantics */
static void X509_free(X509 *x) {
    free(x);
}

/* Reproducer of the vulnerable logic from apps/ocsp.c around OPT_ISSUER */
static int ocsp_main(int argc, char **argv) {
    (void)argc;
    (void)argv;

    X509 *issuer = NULL;
    STACK_OF_X509 *issuers = NULL;

    /* Simulate handling of: -issuer <file> */
    issuer = load_cert("dummy.pem", FORMAT_UNDEF, "issuer certificate");
    if (issuer == NULL)
        goto end;

    if (issuers == NULL) {
        issuers = sk_X509_new_null();
        if (issuers == NULL)
            goto end;
    }

    if (!sk_X509_push(issuers, issuer))
        goto end;

    /* Vulnerability: issuer is pushed onto issuers but not set to NULL */
    /* In later cleanup both issuer and elements of issuers are freed */

end:
    /* This cleanup order is sufficient to trigger double-free */
    if (issuers)
        sk_X509_pop_free(issuers, X509_free);

    /* issuer is the same pointer we just freed via sk_X509_pop_free */
    X509_free(issuer); /* ASan should report double-free here */

    return 0;
}

int main(void) {
    /* Minimal argv to conceptually match an invocation with -issuer */
    char *argv[] = { (char *)"repro", (char *)"-issuer", (char *)"dummy.pem", NULL };
    return ocsp_main(3, argv);
}