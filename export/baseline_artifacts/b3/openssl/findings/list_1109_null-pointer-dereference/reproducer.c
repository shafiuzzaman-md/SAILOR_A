#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdint.h>

/* Minimal stubs to emulate the OpenSSL types and APIs used in the snippet */

typedef struct asn1_object_st {
    int nid;
} ASN1_OBJECT;

typedef struct bio_st {
    int dummy;
} BIO;

#define NID_undef 0

/* Globals as used in apps/list.c */
static BIO *bio_out = NULL;
static BIO *bio_err = NULL;

/* Stub: clear OpenSSL error queue */
static void ERR_clear_error(void) { /* no-op */ }

/* Simple object storage for predictable OBJ_nid2obj() */
static ASN1_OBJECT objs[8];

/* Stub: return maximum nid (exclusive upper bound for loop). We choose 2 so i==1 runs once */
static int OBJ_new_nid(int num) {
    (void)num;
    return 2; /* Loop will run for i = 1 only */
}

/* Stub: map nid to object pointer */
static const ASN1_OBJECT *OBJ_nid2obj(int nid) {
    if (nid < 0 || nid >= (int)(sizeof(objs)/sizeof(objs[0])))
        nid = 0;
    objs[nid].nid = nid;
    return &objs[nid];
}

/* Stub: return short name. Return NULL for nid==1 to trigger the bug */
static const char *OBJ_nid2sn(int nid) {
    if (nid == 1)
        return NULL; /* This is the crasher: %s gets a NULL */
    return "short-name";
}

/* Stub: return long name. Return NULL for nid==1 to trigger the bug */
static const char *OBJ_nid2ln(int nid) {
    if (nid == 1)
        return NULL; /* This is the crasher: %s gets a NULL */
    return "long-name";
}

/* Stub: convert obj to nid (ensure not NID_undef so loop doesn't skip) */
static int OBJ_obj2nid(const ASN1_OBJECT *obj) {
    (void)obj;
    return 1; /* Always defined */
}

/* Stub: convert obj to text. Return 0 so the vulnerable branch is taken */
static int OBJ_obj2txt(char *buf, int buf_len, const ASN1_OBJECT *obj, int always_return) {
    (void)buf; (void)buf_len; (void)obj; (void)always_return;
    return 0; /* Triggers the '# None-OID object' printf path */
}

/* Stubs for OPENSSL memory wrappers (not used on the crashing path) */
static void *OPENSSL_realloc(void *ptr, size_t size) { return realloc(ptr, size); }
static void OPENSSL_free(void *ptr) { free(ptr); }

/* Vulnerable-style BIO_printf that will dereference %s arguments explicitly */
static int BIO_printf(BIO *b, const char *format, ...) {
    (void)b;
    va_list ap;
    va_start(ap, format);

    /* Scan format to find a %s and intentionally dereference it to emulate the crash */
    for (const char *p = format; *p; ++p) {
        if (*p == '%') {
            ++p;
            if (*p == 's') {
                const char *s = va_arg(ap, const char *);
                /* Intentional dereference of possibly NULL pointer to trigger ASan crash */
                volatile char c = s[0];
                (void)c;
                break; /* We do not need to proceed further */
            } else if (*p == '%') {
                /* Literal '%' consumes no args */
            } else {
                /* For simplicity, consume one argument for common specifiers to keep va_list in sync if ever needed */
                if (*p == 'd' || *p == 'i' || *p == 'u' || *p == 'x' || *p == 'X' || *p == 'p') {
                    (void)va_arg(ap, void *);
                } else if (*p == 'f' || *p == 'g' || *p == 'e' || *p == 'E' || *p == 'G' || *p == 'F') {
                    (void)va_arg(ap, double);
                }
            }
        }
    }

    va_end(ap);
    return 0;
}

/* The vulnerable function, adapted from apps/list.c lines 1084-1132 */
static void list_objects(void) {
    int max_nid = OBJ_new_nid(0);
    int i;
    char *oid_buf = NULL;
    int oid_size = 0;

    /* Skip 0, since that's NID_undef */
    for (i = 1; i < max_nid; i++) {
        const ASN1_OBJECT *obj = OBJ_nid2obj(i);
        const char *sn = OBJ_nid2sn(i);
        const char *ln = OBJ_nid2ln(i);
        int n = 0;

        /* Ignore errors from retrieval */
        ERR_clear_error();

        if (OBJ_obj2nid(obj) == NID_undef)
            continue;

        if ((n = OBJ_obj2txt(NULL, 0, obj, 1)) == 0) {
            /* Vulnerable call: sn/ln can be NULL and are passed as %s */
            BIO_printf(bio_out, "# None-OID object: %s, %s\n", sn, ln);
            continue;
        }
        if (n < 0)
            break; /* Error */

        if (n > oid_size) {
            oid_buf = OPENSSL_realloc(oid_buf, n + 1);
            if (oid_buf == NULL) {
                BIO_printf(bio_err, "ERROR: Memory allocation\n");
                break; /* Error */
            }
            oid_size = n + 1;
        }
        if (OBJ_obj2txt(oid_buf, oid_size, obj, 1) < 0)
            break; /* Error */
        if (ln == NULL || strcmp(sn, ln) == 0)
            BIO_printf(bio_out, "%s = %s\n", sn, oid_buf);
        else
            BIO_printf(bio_out, "%s = %s, %s\n", sn, ln, oid_buf);
    }

    OPENSSL_free(oid_buf);
}

int main(void) {
    /* Initialize fake BIOs */
    bio_out = (BIO *)malloc(sizeof(BIO));
    bio_err = (BIO *)malloc(sizeof(BIO));
    if (!bio_out || !bio_err) {
        fprintf(stderr, "Failed to allocate BIOs\n");
        return 1;
    }

    /* This will crash due to NULL sn/ln passed to BIO_printf with %s */
    list_objects();

    free(bio_out);
    free(bio_err);
    return 0;
}
