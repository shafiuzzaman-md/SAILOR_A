// Standalone C reproducer for null-pointer-dereference in collect_signatures
// It simulates sk_EVP_SIGNATURE_new() returning NULL, leading to passing a NULL
// stack into collect_signatures(), where sk_EVP_SIGNATURE_push() dereferences it.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Minimal type stubs to mimic the relevant OpenSSL API

typedef struct evp_signature_st {
    int refcnt;
} EVP_SIGNATURE;

// Emulate STACK_OF(EVP_SIGNATURE)
#define STACK_OF(type) struct stack_st_##type
struct stack_st_EVP_SIGNATURE {
    int num;
    EVP_SIGNATURE **data;
};

// Forward declarations matching the vulnerable path
static int signature_cmp(const EVP_SIGNATURE *const *a, const EVP_SIGNATURE *const *b);
static int is_signature_fetchable(EVP_SIGNATURE *sig);
static int EVP_SIGNATURE_up_ref(EVP_SIGNATURE *sig);
static void EVP_SIGNATURE_free(EVP_SIGNATURE *sig);
static STACK_OF(EVP_SIGNATURE) *sk_EVP_SIGNATURE_new(int (*cmp)(const EVP_SIGNATURE *const *, const EVP_SIGNATURE *const *));
static int sk_EVP_SIGNATURE_push(STACK_OF(EVP_SIGNATURE) *st, EVP_SIGNATURE *val);
static void *app_get0_libctx(void);
static void EVP_SIGNATURE_do_all_provided(void *libctx, void (*fn)(EVP_SIGNATURE *, void *), void *arg);

// Vulnerable callback as in apps/list.c
static void collect_signatures(EVP_SIGNATURE *sig, void *stack) {
    STACK_OF(EVP_SIGNATURE) *sig_stack = stack;

    if (is_signature_fetchable(sig)
        && EVP_SIGNATURE_up_ref(sig)
        && sk_EVP_SIGNATURE_push(sig_stack, sig) <= 0)
        EVP_SIGNATURE_free(sig); /* up-ref successful but push to stack failed */
}

// Minimal version of list_signatures() that reaches the vulnerable path
static void list_signatures(void) {
    // sk_EVP_SIGNATURE_new() is stubbed to return NULL to simulate allocation failure
    STACK_OF(EVP_SIGNATURE) *sig_stack = sk_EVP_SIGNATURE_new(signature_cmp);

    // sig_stack is NULL here, but is passed into the iteration callback
    EVP_SIGNATURE_do_all_provided(app_get0_libctx(), collect_signatures, sig_stack);

    // No further code needed; the crash happens inside collect_signatures -> sk_EVP_SIGNATURE_push
}

// ---------------- Stubs to drive the path ----------------
static int signature_cmp(const EVP_SIGNATURE *const *a, const EVP_SIGNATURE *const *b) {
    (void)a; (void)b; return 0;
}

static int is_signature_fetchable(EVP_SIGNATURE *sig) {
    (void)sig; return 1; // Always treat as fetchable
}

static int EVP_SIGNATURE_up_ref(EVP_SIGNATURE *sig) {
    if (sig) sig->refcnt++;
    return 1; // Succeed
}

static void EVP_SIGNATURE_free(EVP_SIGNATURE *sig) {
    // In this minimal stub we just free the object
    free(sig);
}

// Simulate allocation failure: return NULL
static STACK_OF(EVP_SIGNATURE) *sk_EVP_SIGNATURE_new(int (*cmp)(const EVP_SIGNATURE *const *, const EVP_SIGNATURE *const *)) {
    (void)cmp; // Unused in this stub
    return NULL; // Critical for triggering the bug
}

// This intentionally dereferences the stack, which will be NULL in this reproducer
static int sk_EVP_SIGNATURE_push(STACK_OF(EVP_SIGNATURE) *st, EVP_SIGNATURE *val) {
    // The following line will dereference a NULL pointer when st == NULL
    st->num += 1; // Boom: null-pointer-dereference
    // Not reached; return value shape consistent with OpenSSL's push API
    (void)val;
    return st->num;
}

static void *app_get0_libctx(void) {
    return NULL; // Unused context in this stub
}

// Calls the provided callback at least once to trigger the vulnerable path
static void EVP_SIGNATURE_do_all_provided(void *libctx, void (*fn)(EVP_SIGNATURE *, void *), void *arg) {
    (void)libctx;
    // Create a dummy signature object
    EVP_SIGNATURE *sig = (EVP_SIGNATURE *)calloc(1, sizeof(*sig));
    if (!sig) {
        fprintf(stderr, "Failed to allocate dummy EVP_SIGNATURE\n");
        return;
    }
    // This will call collect_signatures with arg == NULL, leading to NPD in push
    fn(sig, arg);
}

int main(void) {
    fprintf(stderr, "Triggering collect_signatures with NULL stack...\n");
    list_signatures();
    // If we got here without crashing (unexpected), exit non-zero
    fprintf(stderr, "Unexpectedly survived null-pointer dereference path.\n");
    return 1;
}
