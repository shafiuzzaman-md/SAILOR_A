// Standalone C reproducer for NULL pointer dereference in collect_kem
// Compile: clang -fsanitize=address -g -O0 reproducer.c -o reproducer

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Minimal opaque types to satisfy signatures
typedef struct ossl_lib_ctx_st OSSL_LIB_CTX;
typedef struct evp_kem_st EVP_KEM;
typedef struct provider_st OSSL_PROVIDER;

// Emulate OpenSSL's STACK_OF(EVP_KEM) machinery just enough for this reproducer
#define STACK_OF(x) STACK_##x

typedef struct {
    int n;
} STACK_EVP_KEM;

// Forward declarations matching the shapes used in the target code
static int kem_cmp(const EVP_KEM *const *a, const EVP_KEM *const *b);
static void collect_kem(EVP_KEM *kem, void *stack);

// Stub implementations of APIs referenced by the vulnerable code path
static int is_kem_fetchable(EVP_KEM *kem) {
    (void)kem;
    return 1; // Always claim it's fetchable
}

static int EVP_KEM_up_ref(EVP_KEM *kem) {
    (void)kem;
    return 1; // Pretend refcount increment succeeded
}

static void EVP_KEM_free(EVP_KEM *kem) {
    free(kem);
}

// Provider/name helpers used by kem_cmp (not actually exercised due to early crash)
static const char *OSSL_PROVIDER_get0_name(const OSSL_PROVIDER *prov) {
    (void)prov;
    return "stub-provider";
}

static const OSSL_PROVIDER *EVP_KEM_get0_provider(const EVP_KEM *k) {
    (void)k;
    return (const OSSL_PROVIDER *)0x1; // Non-NULL dummy
}

static int kem_cmp(const EVP_KEM *const *a, const EVP_KEM *const *b) {
    return strcmp(OSSL_PROVIDER_get0_name(EVP_KEM_get0_provider(*a)),
                  OSSL_PROVIDER_get0_name(EVP_KEM_get0_provider(*b)));
}

// Simulate OpenSSL's do_all which calls the callback with the provided arg
static void EVP_KEM_do_all_provided(OSSL_LIB_CTX *libctx,
                                    void (*fn)(EVP_KEM *kem, void *arg),
                                    void *arg) {
    (void)libctx;
    EVP_KEM *kem = (EVP_KEM *)malloc(sizeof(*kem));
    if (kem == NULL) {
        perror("malloc");
        exit(1);
    }
    // Invoke the callback once, passing through the (possibly NULL) stack arg
    fn(kem, arg);
    // If callback didn't take ownership, free here to keep things tidy
    // (In our crash case, we never reach this line.)
    free(kem);
}

// Application context accessor used by list_kems
static OSSL_LIB_CTX *app_get0_libctx(void) {
    return NULL; // Not relevant for the reproducer
}

// STACK functions - new returns NULL to simulate allocation failure in list_kems
static STACK_EVP_KEM *sk_EVP_KEM_new(int (*cmp)(const EVP_KEM *const *, const EVP_KEM *const *)) {
    (void)cmp;
    // Critical to reproducer: return NULL to simulate OOM in sk_EVP_KEM_new
    return NULL;
}

// This push will dereference the stack pointer and thus crash when it's NULL
static int sk_EVP_KEM_push(STACK_EVP_KEM *sk, EVP_KEM *val) {
    if (val == NULL) return 0;
    // Intentional NULL dereference when sk == NULL to mirror OpenSSL's behavior
    sk->n += 1; // ASan will catch this as a NULL pointer dereference
    return sk->n;
}

// Vulnerable callback copied/reshaped from the source
static void collect_kem(EVP_KEM *kem, void *stack) {
    STACK_OF(EVP_KEM) *kem_stack = (STACK_OF(EVP_KEM) *)stack;

    if (is_kem_fetchable(kem)
        && EVP_KEM_up_ref(kem)
        && sk_EVP_KEM_push(kem_stack, kem) <= 0)
        EVP_KEM_free(kem); // not reached in this reproducer
}

// Minimal version of list_kems that exercises the vulnerable path
static void list_kems(void) {
    // Returns NULL (simulated allocation failure)
    STACK_OF(EVP_KEM) *kem_stack = sk_EVP_KEM_new(kem_cmp);

    // Pass the NULL stack into the callback via do_all_provided
    EVP_KEM_do_all_provided(app_get0_libctx(), collect_kem, kem_stack);
}

int main(void) {
    // Trigger the vulnerability: collect_kem will attempt to push onto a NULL stack
    list_kems();
    return 0;
}
