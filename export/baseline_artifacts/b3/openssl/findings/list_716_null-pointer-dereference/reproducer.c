// Standalone C reproducer for null-pointer-dereference in collect_skeymanagers
// It mimics the vulnerable OpenSSL apps/list.c code path where
// sk_EVP_SKEYMGMT_new() returns NULL and that NULL is used in
// collect_skeymanagers() via sk_EVP_SKEYMGMT_push().

#include <stdio.h>
#include <stdlib.h>

// Minimal stand-ins for OpenSSL's STACK_OF macro and types
#define STACK_OF(type) struct stack_st_##type

typedef struct evp_skeymgmt_st {
    int refcnt;
} EVP_SKEYMGMT;

struct stack_st_EVP_SKEYMGMT {
    int n;
};

// Stub helper: always report fetchable
static int is_skeymgmt_fetchable(EVP_SKEYMGMT *km) {
    (void)km;
    return 1;
}

// Stub: increments ref (no-op)
static void EVP_SKEYMGMT_up_ref(EVP_SKEYMGMT *km) {
    if (km) km->refcnt++;
}

// Vulnerable push: dereferences the stack pointer (will crash if NULL)
static int sk_EVP_SKEYMGMT_push(STACK_OF(EVP_SKEYMGMT) *st, EVP_SKEYMGMT *km) {
    // This dereference makes the NULL bug visible under ASan
    st->n += 1;  // If st == NULL, this is a null dereference
    (void)km;
    return st->n;
}

// Comparator (unused in this reproducer, but matches signature)
static int skeymanager_cmp(const EVP_SKEYMGMT *const *a,
                           const EVP_SKEYMGMT *const *b) {
    (void)a; (void)b;
    return 0;
}

// Stub: simulate allocation failure -> returns NULL
static STACK_OF(EVP_SKEYMGMT) *sk_EVP_SKEYMGMT_new(
    int (*cmp)(const EVP_SKEYMGMT *const *, const EVP_SKEYMGMT *const *)) {
    (void)cmp;
    return NULL;  // Critical: forces km_stack to be NULL
}

// Stub: app libctx provider handle (unused)
static void *app_get0_libctx(void) {
    return NULL;
}

// Callback collection function (matches vulnerable code path)
static void collect_skeymanagers(EVP_SKEYMGMT *km, void *stack) {
    STACK_OF(EVP_SKEYMGMT) *km_stack = stack;

    // Line modeled after the vulnerable code:
    // if (is_skeymgmt_fetchable(km)
    //     && sk_EVP_SKEYMGMT_push(km_stack, km) > 0)
    //     EVP_SKEYMGMT_up_ref(km);
    if (is_skeymgmt_fetchable(km)
        && sk_EVP_SKEYMGMT_push(km_stack, km) > 0)
        EVP_SKEYMGMT_up_ref(km);
}

// Iterate over provided implementations and call the collector
static void EVP_SKEYMGMT_do_all_provided(void *libctx,
                                         void (*fn)(EVP_SKEYMGMT *, void *),
                                         void *stack) {
    (void)libctx;
    EVP_SKEYMGMT dummy = {0};
    // Call the callback once with a valid km and the (possibly NULL) stack
    fn(&dummy, stack);
}

// Minimal list_skeymanagers that triggers the bug early
static void list_skeymanagers(void) {
    // km_stack becomes NULL due to simulated allocation failure
    STACK_OF(EVP_SKEYMGMT) *km_stack = sk_EVP_SKEYMGMT_new(skeymanager_cmp);

    // This passes the NULL km_stack into the callback, which then calls
    // sk_EVP_SKEYMGMT_push(km_stack, ...), dereferencing NULL.
    EVP_SKEYMGMT_do_all_provided(app_get0_libctx(), collect_skeymanagers, km_stack);
}

int main(void) {
    // Trigger the vulnerable path
    list_skeymanagers();
    // If we get here, the bug didn't trigger
    puts("Did not trigger (unexpected)");
    return 0;
}
