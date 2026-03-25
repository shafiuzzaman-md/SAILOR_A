// Standalone C reproducer for NULL pointer dereference in collect_keymanagers/list_keymanagers
// Compile with:
//   clang -fsanitize=address -g -O0 reproducer.c -o reproducer

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Minimal stand-ins for OpenSSL types/APIs used by the vulnerable code

typedef struct evp_keymgmt_st {
    int refcnt;
} EVP_KEYMGMT;

// Emulate OpenSSL's STACK_OF(EVP_KEYMGMT) machinery just enough for this repro
#define STACK_OF(x) STACK_OF_##x

typedef struct {
    int num;
    int cap;
    EVP_KEYMGMT **items;
    int (*cmp)(const EVP_KEYMGMT *const *, const EVP_KEYMGMT *const *);
} STACK_OF_EVP_KEYMGMT;

// Stub: always says the keymgmt is fetchable
static int is_keymgmt_fetchable(EVP_KEYMGMT *km) {
    (void)km;
    return 1;
}

// Stub: always succeeds up_ref
static int EVP_KEYMGMT_up_ref(EVP_KEYMGMT *km) {
    if (km) km->refcnt++;
    return 1;
}

static void EVP_KEYMGMT_free(EVP_KEYMGMT *km) {
    (void)km;
}

// Stub: returns NULL to simulate allocation failure in sk_EVP_KEYMGMT_new
static STACK_OF(EVP_KEYMGMT) *sk_EVP_KEYMGMT_new(
    int (*cmp)(const EVP_KEYMGMT *const *, const EVP_KEYMGMT *const *))
{
    (void)cmp;
    // Simulate OOM: return NULL so downstream code mishandles it
    return NULL;
}

// This function will be called with a NULL 'sk' due to the bug in list_keymanagers
// and will therefore dereference a NULL pointer, triggering the crash.
static int sk_EVP_KEYMGMT_push(STACK_OF(EVP_KEYMGMT) *sk, EVP_KEYMGMT *val) {
    // Intentional: this will dereference 'sk' even if it's NULL (as in the buggy path)
    if (sk->num >= sk->cap) {  // NULL deref happens here
        int newcap = sk->cap == 0 ? 4 : sk->cap * 2;
        EVP_KEYMGMT **newitems = (EVP_KEYMGMT **)realloc(sk->items, newcap * sizeof(*newitems));
        if (!newitems) return 0;
        sk->items = newitems;
        sk->cap = newcap;
    }
    sk->items[sk->num++] = val;
    return 1;
}

// Also dereferences the stack (and would crash) if reached with NULL
static void sk_EVP_KEYMGMT_sort(STACK_OF(EVP_KEYMGMT) *sk) {
    // Touch sk->num to ensure a NULL deref if this is reached
    if (sk->num > 1) {
        // no-op sort stub
    }
}

static int sk_EVP_KEYMGMT_num(const STACK_OF(EVP_KEYMGMT) *sk) {
    return sk ? sk->num : 0;
}

static EVP_KEYMGMT *sk_EVP_KEYMGMT_value(const STACK_OF(EVP_KEYMGMT) *sk, int idx) {
    if (!sk || idx < 0 || idx >= sk->num) return NULL;
    return sk->items[idx];
}

// Stub: application libctx accessor
static void *app_get0_libctx(void) { return NULL; }

// Stub: walk all provided keymanagers; here we just call the callback once
static void EVP_KEYMGMT_do_all_provided(void *libctx,
                                        void (*cb)(EVP_KEYMGMT *km, void *arg),
                                        void *arg)
{
    (void)libctx;
    EVP_KEYMGMT dummy = { .refcnt = 1 };
    cb(&dummy, arg);
}

// Comparator with the expected signature; content doesn't matter for the repro
static int keymanager_cmp(const EVP_KEYMGMT *const *a,
                          const EVP_KEYMGMT *const *b)
{
    (void)a; (void)b;
    return 0;
}

// Vulnerable helper: receives a NULL stack in the buggy path and calls sk_EVP_KEYMGMT_push
static void collect_keymanagers(EVP_KEYMGMT *km, void *stack)
{
    STACK_OF(EVP_KEYMGMT) *km_stack = (STACK_OF(EVP_KEYMGMT) *)stack;

    if (is_keymgmt_fetchable(km)
        && EVP_KEYMGMT_up_ref(km)
        && sk_EVP_KEYMGMT_push(km_stack, km) <= 0)
        EVP_KEYMGMT_free(km); /* up-ref successful but push to stack failed */
}

// Vulnerable function under test: fails to check sk_EVP_KEYMGMT_new() for NULL
static void list_keymanagers(void)
{
    STACK_OF(EVP_KEYMGMT) *km_stack = sk_EVP_KEYMGMT_new(keymanager_cmp);

    // km_stack is NULL here due to our stub; following calls misuse it
    EVP_KEYMGMT_do_all_provided(app_get0_libctx(), collect_keymanagers, km_stack);
    // Even if the above didn't crash, this unconditional sort on NULL would
    sk_EVP_KEYMGMT_sort(km_stack);
}

int main(void)
{
    fprintf(stderr, "Triggering NULL pointer dereference in list_keymanagers...\n");
    // This call will crash inside collect_keymanagers -> sk_EVP_KEYMGMT_push(NULL, ...)
    list_keymanagers();
    // Not reached
    return 0;
}
