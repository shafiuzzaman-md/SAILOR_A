// Standalone reproducer for null-pointer-dereference in collect_asymciph
// Mirrors apps/list.c: collect_asymciph() being called with a NULL stack
// due to unchecked sk_EVP_ASYM_CIPHER_new() allocation failure in list_asymciphers().

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Minimal stand-ins for OpenSSL types/APIs used in the vulnerable code

typedef struct evp_asym_cipher {
    int refcnt;
} EVP_ASYM_CIPHER;

// Simulated STACK_OF(EVP_ASYM_CIPHER)
typedef struct stack_evp_asym_cipher {
    size_t n;
    size_t cap;
    EVP_ASYM_CIPHER **data;
} STACK_OF_EVP_ASYM_CIPHER;

// Comparator prototype to match sk_EVP_ASYM_CIPHER_new signature
static int asymcipher_cmp(const EVP_ASYM_CIPHER *const *a,
                          const EVP_ASYM_CIPHER *const *b) {
    (void)a; (void)b;
    return 0;
}

// Simulate allocation failure: returns NULL to mirror the bug trigger path
static STACK_OF_EVP_ASYM_CIPHER *sk_EVP_ASYM_CIPHER_new(
    int (*cmp)(const EVP_ASYM_CIPHER *const *, const EVP_ASYM_CIPHER *const *))
{
    (void)cmp;
    return NULL; // Intentionally fail allocation
}

// This push function intentionally dereferences the stack pointer.
// If 'st' is NULL, this will crash (as it would inside the real push).
static int sk_EVP_ASYM_CIPHER_push(STACK_OF_EVP_ASYM_CIPHER *st,
                                   EVP_ASYM_CIPHER *val)
{
    // The following line dereferences 'st' and will cause a null-pointer-dereference
    // when 'st' is NULL, which matches the vulnerable call site behavior.
    st->n++;
    (void)val;
    return (int)st->n;
}

// Stubs to satisfy collect_asymciph logic
static int is_asym_cipher_fetchable(EVP_ASYM_CIPHER *asym_cipher) {
    (void)asym_cipher;
    return 1; // Force true to reach the vulnerable push()
}

static int EVP_ASYM_CIPHER_up_ref(EVP_ASYM_CIPHER *asym_cipher) {
    if (asym_cipher == NULL)
        return 0;
    asym_cipher->refcnt++;
    return 1; // Force true to reach the vulnerable push()
}

static void EVP_ASYM_CIPHER_free(EVP_ASYM_CIPHER *asym_cipher) {
    free(asym_cipher);
}

// Vulnerable callback (mirrors apps/list.c: collect_asymciph)
static void collect_asymciph(EVP_ASYM_CIPHER *asym_cipher, void *stack)
{
    STACK_OF_EVP_ASYM_CIPHER *asym_cipher_stack = (STACK_OF_EVP_ASYM_CIPHER *)stack;

    if (is_asym_cipher_fetchable(asym_cipher)
        && EVP_ASYM_CIPHER_up_ref(asym_cipher)
        // This call dereferences asym_cipher_stack even when it's NULL
        && sk_EVP_ASYM_CIPHER_push(asym_cipher_stack, asym_cipher) <= 0)
        EVP_ASYM_CIPHER_free(asym_cipher); // unreachable due to crash above
}

// Stubbed libctx access
static void *app_get0_libctx(void) { return NULL; }

typedef void (*evp_asym_cb)(EVP_ASYM_CIPHER *, void *);

// Simulate provider iteration that invokes the callback at least once
static void EVP_ASYM_CIPHER_do_all_provided(void *libctx, evp_asym_cb cb, void *arg)
{
    (void)libctx;
    EVP_ASYM_CIPHER *c = (EVP_ASYM_CIPHER *)calloc(1, sizeof(*c));
    if (c == NULL) {
        perror("calloc");
        exit(1);
    }
    cb(c, arg); // This will crash inside collect_asymciph due to NULL stack
    // Not reached if crash occurs
    EVP_ASYM_CIPHER_free(c);
}

// Minimal reproduction of list_asymciphers() focusing on the buggy sequence
static void list_asymciphers(void)
{
    // Allocation failure path: returns NULL but not checked
    STACK_OF_EVP_ASYM_CIPHER *asymciph_stack = sk_EVP_ASYM_CIPHER_new(asymcipher_cmp);

    // Pass the (possibly NULL) stack to the callback via iterator
    EVP_ASYM_CIPHER_do_all_provided(app_get0_libctx(), collect_asymciph, asymciph_stack);
}

int main(void)
{
    // Trigger the vulnerable path deterministically
    list_asymciphers();
    return 0;
}
