#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

/* Minimal stub types and functions to mimic the OpenSSL API used */

typedef struct asn1_object_st {
    int nid;
} ASN1_OBJECT;

#define NID_undef 0

/* Fake BIO implementation */
typedef struct fake_bio_st {
    FILE *f;
} BIO;

BIO *bio_out = NULL;
BIO *bio_err = NULL;

static BIO *make_bio(FILE *f) {
    BIO *b = (BIO *)malloc(sizeof(*b));
    if (b != NULL) b->f = f;
    return b;
}

static int BIO_printf(BIO *bio, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int r = vfprintf(bio ? bio->f : stdout, fmt, ap);
    va_end(ap);
    return r;
}

/* Error API stub */
static void ERR_clear_error(void) {
    /* no-op */
}

/* Memory API stubs */
static void *OPENSSL_realloc(void *p, size_t n) { return realloc(p, n); }
static void OPENSSL_free(void *p) { free(p); }

/* OBJ API stubs */
static int OBJ_new_nid(int x) {
    (void)x;
    /* Return an artificial max NID (>1 so the loop runs). */
    return 3; /* will iterate i = 1, 2 */
}

static ASN1_OBJECT *get_obj_slot(int nid) {
    static ASN1_OBJECT objs[4]; /* enough for nid up to 3 */
    if (nid >= 0 && nid < 4) {
        objs[nid].nid = nid;
        return &objs[nid];
    }
    return NULL;
}

static const ASN1_OBJECT *OBJ_nid2obj(int nid) {
    return get_obj_slot(nid);
}

static int OBJ_obj2nid(const ASN1_OBJECT *obj) {
    if (obj == NULL) return NID_undef;
    return obj->nid; /* non-zero for our test nids */
}

static const char *OBJ_nid2sn(int nid) {
    /* Return NULL for nid == 1 to trigger the bug; non-NULL otherwise. */
    if (nid == 1) return NULL;            /* Short name is missing */
    if (nid == 2) return "SN2";
    return "SNX";
}

static const char *OBJ_nid2ln(int nid) {
    if (nid == 1) return "LongOnly";     /* Only long name exists */
    if (nid == 2) return "Long2";
    return "LongX";
}

static int OBJ_obj2txt(char *buf, int buf_len, const ASN1_OBJECT *obj, int no_name) {
    (void)obj;
    (void)no_name;
    /* Return a fixed OID string; if buf is NULL, return required length.
       This mimics the real API's behavior pattern used in the code. */
    const char *oid = "1.2.840.113549";
    int len = (int)strlen(oid);
    if (buf == NULL || buf_len <= 0)
        return len; /* required size */
    /* Write into buffer if provided */
    if (buf_len > 0) {
        int n = (len < buf_len - 1) ? len : (buf_len - 1);
        memcpy(buf, oid, n);
        buf[n] = '\0';
    }
    return len;
}

/* The vulnerable function from apps/list.c, adapted to use our stubs */
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

        /* Ignore any errors from retrieval */
        ERR_clear_error();

        if (OBJ_obj2nid(obj) == NID_undef)
            continue;

        if ((n = OBJ_obj2txt(NULL, 0, obj, 1)) == 0) {
            BIO_printf(bio_out, "# None-OID object: %s, %s\n", sn, ln);
            continue;
        }
        if (n < 0)
            break; /* Error */

        if (n > oid_size) {
            oid_buf = (char *)OPENSSL_realloc(oid_buf, n + 1);
            if (oid_buf == NULL) {
                BIO_printf(bio_err, "ERROR: Memory allocation\n");
                break; /* Error */
            }
            oid_size = n + 1;
        }
        if (OBJ_obj2txt(oid_buf, oid_size, obj, 1) < 0)
            break; /* Error */

        /* Vulnerable lines: sn may be NULL; strcmp(NULL, ln) dereferences NULL */
        if (ln == NULL || strcmp(sn, ln) == 0)
            BIO_printf(bio_out, "%s = %s\n", sn, oid_buf);
        else
            BIO_printf(bio_out, "%s = %s, %s\n", sn, ln, oid_buf);
    }

    OPENSSL_free(oid_buf);
}

int main(void) {
    bio_out = make_bio(stdout);
    bio_err = make_bio(stderr);

    /* This will iterate over NIDs 1..(max_nid-1). For nid=1, OBJ_nid2sn() returns
       NULL while OBJ_nid2ln() returns a non-NULL long name, causing strcmp(NULL, ln)
       to crash. */
    list_objects();

    /* Cleanup */
    free(bio_out);
    free(bio_err);
    return 0;
}