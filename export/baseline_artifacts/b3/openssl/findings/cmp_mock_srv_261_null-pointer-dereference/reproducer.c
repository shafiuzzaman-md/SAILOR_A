#include <stdio.h>
#include <stdlib.h>

/* Minimal stand-ins for OpenSSL types */
typedef struct x509_name_st {
    int dummy; /* any field to allow dereference */
} X509_NAME;

typedef struct asn1_integer_st {
    int value;
} ASN1_INTEGER;

typedef struct x509_st {
    const X509_NAME *issuer;
    const ASN1_INTEGER *serial;
} X509;

/* Stubs mimicking the OpenSSL API used by the vulnerable function */
static const X509_NAME *X509_get_issuer_name(const X509 *x)
{
    return x ? x->issuer : NULL;
}

static const ASN1_INTEGER *X509_get0_serialNumber(const X509 *x)
{
    return x ? x->serial : NULL;
}

/* Deliberately dereferences the first argument, causing a NULL-deref
 * when called with a NULL issuer as in the buggy code path. */
static int X509_NAME_cmp(const X509_NAME *a, const X509_NAME *b)
{
    /* This will crash if 'a' is NULL, which models the real library's
     * expectation that inputs are non-NULL and exposes the bug. */
    int ad = ((const struct x509_name_st *)a)->dummy;
    int bd = ((const struct x509_name_st *)b)->dummy;
    if (ad == bd)
        return 0;
    return (ad < bd) ? -1 : 1;
}

static int ASN1_INTEGER_cmp(const ASN1_INTEGER *a, const ASN1_INTEGER *b)
{
    if (a == NULL || b == NULL) {
        /* Simple behavior for this reproducer; not used in the crash */
        return (a == b) ? 0 : (a ? 1 : -1);
    }
    if (a->value == b->value)
        return 0;
    return (a->value < b->value) ? -1 : 1;
}

/* Vulnerable function copied from apps/lib/cmp_mock_srv.c (simplified types) */
static int refcert_cmp(const X509 *refcert,
                       const X509_NAME *issuer, const ASN1_INTEGER *serial)
{
    const X509_NAME *ref_issuer;
    const ASN1_INTEGER *ref_serial;

    if (refcert == NULL)
        return 1;
    ref_issuer = X509_get_issuer_name(refcert);
    ref_serial = X509_get0_serialNumber(refcert);
    /* BUG: checks ref_issuer instead of issuer. If issuer is NULL and
     * ref_issuer != NULL, X509_NAME_cmp(issuer, ref_issuer) dereferences NULL */
    return (ref_issuer == NULL || X509_NAME_cmp(issuer, ref_issuer) == 0)
        && (ref_serial == NULL || ASN1_INTEGER_cmp(serial, ref_serial) == 0);
}

int main(void)
{
    /* Build a reference cert with a non-NULL issuer so the buggy branch calls X509_NAME_cmp */
    X509_NAME *non_null_issuer = (X509_NAME *)malloc(sizeof(*non_null_issuer));
    if (!non_null_issuer) {
        perror("malloc");
        return 1;
    }
    non_null_issuer->dummy = 42;

    X509 refcert;
    refcert.issuer = non_null_issuer;  /* ensures ref_issuer != NULL */
    refcert.serial = NULL;             /* serial not relevant for trigger */

    const X509_NAME *issuer_missing = NULL;     /* simulate omitted issuer field in request */
    const ASN1_INTEGER *serial_missing = NULL;  /* also omitted */

    /* This call will evaluate X509_NAME_cmp(issuer_missing, ref_issuer),
     * which dereferences a NULL pointer and triggers ASan */
    int res = refcert_cmp(&refcert, issuer_missing, serial_missing);

    /* Should not reach here due to crash */
    printf("refcert_cmp returned %d\n", res);

    free(non_null_issuer);
    return 0;
}
