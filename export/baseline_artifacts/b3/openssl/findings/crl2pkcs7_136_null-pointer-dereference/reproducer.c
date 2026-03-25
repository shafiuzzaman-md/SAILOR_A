#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Minimal stub type definitions to mirror OpenSSL structures */
typedef struct asn1_object_st {
    int nid;
} ASN1_OBJECT;

typedef struct pkcs7_st PKCS7;
typedef struct pkcs7_signed_st PKCS7_SIGNED;

struct pkcs7_st {
    ASN1_OBJECT *type;
    union {
        PKCS7_SIGNED *sign;
        void *other;
    } d;
};

struct pkcs7_signed_st {
    void *version;      /* not used in this reproducer */
    PKCS7 *contents;    /* BUG: stays NULL after PKCS7_SIGNED_new() */
    void *crl;          /* not used */
    void *cert;         /* not used */
};

/* Minimal stubbed API to mirror OpenSSL behavior */
static PKCS7 *PKCS7_new(void) {
    return (PKCS7 *)calloc(1, sizeof(PKCS7));
}

static PKCS7_SIGNED *PKCS7_SIGNED_new(void) {
    /* contents intentionally left NULL, matching vulnerable initialization */
    return (PKCS7_SIGNED *)calloc(1, sizeof(PKCS7_SIGNED));
}

static ASN1_OBJECT *OBJ_nid2obj(int nid) {
    ASN1_OBJECT *o = (ASN1_OBJECT *)malloc(sizeof(ASN1_OBJECT));
    if (o) o->nid = nid;
    return o;
}

/* NID stubs */
#define NID_pkcs7_signed 1
#define NID_pkcs7_data   2

/* Vulnerable function replica from apps/crl2pkcs7.c */
int crl2pkcs7_main(int argc, char **argv) {
    (void)argc; (void)argv;

    PKCS7 *p7 = PKCS7_new();
    PKCS7_SIGNED *p7s = PKCS7_SIGNED_new();

    if (p7 == NULL || p7s == NULL) {
        fprintf(stderr, "Allocation failed\n");
        return 1;
    }

    /* Setup mimicking the original code path */
    p7->type = OBJ_nid2obj(NID_pkcs7_signed);
    p7->d.sign = p7s;

    /* Crash: p7s->contents is never initialized -> NULL dereference */
    /* This line corresponds to the vulnerable line 136 in crl2pkcs7.c */
    p7s->contents->type = OBJ_nid2obj(NID_pkcs7_data);

    return 0; /* Not reached */
}

int main(int argc, char **argv) {
    return crl2pkcs7_main(argc, argv);
}
