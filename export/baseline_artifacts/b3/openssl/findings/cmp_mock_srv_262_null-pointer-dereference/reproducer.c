#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Minimal stub types to emulate OpenSSL structures */
typedef struct asn1_integer_st {
    int value;
} ASN1_INTEGER;

typedef struct x509_name_st {
    int dummy;
} X509_NAME;

typedef struct x509_st {
    X509_NAME *issuer;
    ASN1_INTEGER *serial;
} X509;

/* Stub implementations of the OpenSSL API used by refcert_cmp */
static const X509_NAME *X509_get_issuer_name(const X509 *x)
{
    return x ? x->issuer : NULL;
}

static const ASN1_INTEGER *X509_get0_serialNumber(const X509 *x)
{
    return x ? x->serial : NULL;
}

static int X509_NAME_cmp(const X509_NAME *a, const X509_NAME *b)
{
    /* Return 0 when equal (same pointer), non-zero otherwise */
    return (a == b) ? 0 : 1;
}

/* Intentionally dereferences inputs (no NULL checks) to mimic OpenSSL behavior */
static int ASN1_INTEGER_cmp(const ASN1_INTEGER *a, const ASN1_INTEGER *b)
{
    /* This will crash with a NULL "a" as in the vulnerable path */
    if (a->value == b->value)
        return 0;
    return (a->value < b->value) ? -1 : 1;
}

/* Vulnerable function exactly mirroring the bug: checks ref_serial instead of serial */
static int refcert_cmp(const X509 *refcert,
                       const X509_NAME *issuer, const ASN1_INTEGER *serial)
{
    const X509_NAME *ref_issuer;
    const ASN1_INTEGER *ref_serial;

    if (refcert == NULL)
        return 1;
    ref_issuer = X509_get_issuer_name(refcert);
    ref_serial = X509_get0_serialNumber(refcert);
    return (ref_issuer == NULL || X509_NAME_cmp(issuer, ref_issuer) == 0)
        && (ref_serial == NULL || ASN1_INTEGER_cmp(serial, ref_serial) == 0);
}

int main(void)
{
    /* Build a reference cert with non-NULL issuer and serial */
    X509_NAME *issuer = (X509_NAME *)malloc(sizeof(*issuer));
    ASN1_INTEGER *ref_serial = (ASN1_INTEGER *)malloc(sizeof(*ref_serial));
    X509 *refcert = (X509 *)malloc(sizeof(*refcert));

    if (!issuer || !ref_serial || !refcert) {
        fprintf(stderr, "Allocation failed\n");
        return 1;
    }

    ref_serial->value = 12345; /* any non-zero value */
    refcert->issuer = issuer;  /* ensure issuer matches to pass first clause */
    refcert->serial = ref_serial; /* ensure ref_serial is non-NULL */

    /* Simulate a request omitting the serial field: pass serial == NULL. */
    const ASN1_INTEGER *missing_serial = NULL;

    /* Triggers the bug: right side evaluates ASN1_INTEGER_cmp(NULL, ref_serial) */
    int res = refcert_cmp(refcert, issuer, missing_serial);

    /* Should not reach here due to crash */
    printf("refcert_cmp returned: %d\n", res);

    /* Cleanup (unreached on crash) */
    free(refcert);
    free(ref_serial);
    free(issuer);

    return 0;
}
