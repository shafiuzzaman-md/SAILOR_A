// Compile with:
//   clang -fsanitize=address -g -O0 reproducer.c -o reproducer
// (The test harness may also add -I/tmp/openssl_upstream and link flags; this
// program is fully self-contained and does not require OpenSSL.)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

/* Minimal stand-ins for OpenSSL types used by apps/cmp.c */
typedef struct asn1_object_st {
    int dummy; /* not used */
} ASN1_OBJECT;

typedef struct ossl_cmp_itav_st {
    ASN1_OBJECT *type; /* accessed by OSSL_CMP_ITAV_get0_type */
} OSSL_CMP_ITAV;

/* A very small substitute for STACK_OF(OSSL_CMP_ITAV) with the accessors used */
typedef struct {
    OSSL_CMP_ITAV **items;
    int num;
} OSSL_STACK_ITAV; /* replacement for STACK_OF(OSSL_CMP_ITAV) */

static int sk_OSSL_CMP_ITAV_num(const OSSL_STACK_ITAV *s) {
    return s == NULL ? 0 : s->num;
}

static OSSL_CMP_ITAV *sk_OSSL_CMP_ITAV_value(const OSSL_STACK_ITAV *s, int idx) {
    if (s == NULL || idx < 0 || idx >= s->num)
        return NULL;
    return s->items[idx];
}

/* Stubs for logging/macros used by apps/cmp.c */
static void CMP_info(const char *msg) {
    fprintf(stderr, "INFO: %s\n", msg);
}

static void CMP_err1(const char *fmt, int a) {
    fprintf(stderr, "ERR: ");
    fprintf(stderr, fmt, a);
    fprintf(stderr, "\n");
}

static void CMP_info2(const char *fmt, int a, const char *b) {
    fprintf(stderr, "INFO: ");
    fprintf(stderr, fmt, a, b);
    fprintf(stderr, "\n");
}

/* Stub for i2t_ASN1_OBJECT used later in print_itavs (won't be reached). */
static int i2t_ASN1_OBJECT(char *buf, size_t len, ASN1_OBJECT *a) {
    (void)a;
    const char *name = "dummy-oid-name";
    if (len == 0) return -1;
    snprintf(buf, len, "%s", name);
    return (int)strlen(name);
}

/* The accessor that dereferences 'itav' unconditionally, mirroring real API */
static ASN1_OBJECT *OSSL_CMP_ITAV_get0_type(OSSL_CMP_ITAV *itav) {
    /* This will cause a NULL-pointer dereference when 'itav' is NULL */
    return itav->type;
}

/* Vulnerable function adapted from apps/cmp.c:print_itavs() */
static int print_itavs(const OSSL_STACK_ITAV *itavs) {
    int i, ret = 1;
    int n = sk_OSSL_CMP_ITAV_num(itavs);

    if (n <= 0) { /* also in case itavs == NULL */
        CMP_info("genp does not contain any ITAV");
        return ret;
    }

    for (i = 1; i <= n; i++) {
        OSSL_CMP_ITAV *itav = sk_OSSL_CMP_ITAV_value(itavs, i - 1);
        /* Bug: 'itav' is used here before checking for NULL */
        ASN1_OBJECT *type = OSSL_CMP_ITAV_get0_type(itav);
        char name[80];

        if (itav == NULL) {
            CMP_err1("could not get ITAV #%d from genp", i);
            ret = 0;
            continue;
        }
        if (i2t_ASN1_OBJECT(name, sizeof(name), type) <= 0) {
            CMP_err1("error parsing type of ITAV #%d from genp", i);
            ret = 0;
        } else {
            CMP_info2("ITAV #%d from genp infoType=%s", i, name);
        }
    }
    return ret;
}

int main(void) {
    /* Construct a stack with a single NULL element to trigger the bug */
    OSSL_CMP_ITAV *arr[1];
    OSSL_STACK_ITAV stack;

    arr[0] = NULL; /* This NULL will be returned by sk_OSSL_CMP_ITAV_value */
    stack.items = arr;
    stack.num = 1;

    fprintf(stderr, "About to trigger NULL dereference in print_itavs...\n");
    /* This call will dereference NULL inside OSSL_CMP_ITAV_get0_type */
    (void)print_itavs(&stack);

    /* We should never reach here due to the crash */
    fprintf(stderr, "Unexpectedly survived.\n");
    return 0;
}
