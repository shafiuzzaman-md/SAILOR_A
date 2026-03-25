#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Minimal stub types emulating the pieces used by apps/lib/cmp_mock_srv.c */

/* ASN1 UTF8 string */
typedef struct {
    unsigned char *data;
    int length;
} ASN1_UTF8STRING;

static const unsigned char *ASN1_STRING_get0_data(const ASN1_UTF8STRING *str) {
    return str ? str->data : NULL;
}

/* ASN1 object carrying just a NID */
typedef struct {
    int nid;
} ASN1_OBJECT;

static int OBJ_obj2nid(const ASN1_OBJECT *obj) {
    return obj ? obj->nid : -1;
}

/* Fake NID for id-it-certProfile */
#define NID_id_it_certProfile 12345

/* Fake CMP constants */
#define OSSL_CMP_CR 1

/* Forward decls */
struct OSSL_CMP_ITAV;

/* Simple stack for ASN1_UTF8STRING */
typedef struct {
    int num;
    ASN1_UTF8STRING **data;
} STACK_OF_ASN1_UTF8STRING;

static int sk_ASN1_UTF8STRING_num(const STACK_OF_ASN1_UTF8STRING *s) {
    return s ? s->num : 0;
}

static ASN1_UTF8STRING *sk_ASN1_UTF8STRING_value(const STACK_OF_ASN1_UTF8STRING *s, int i) {
    if (!s || i < 0 || i >= s->num) return NULL;
    return s->data[i];
}

/* Simple stack for OSSL_CMP_ITAV */
typedef struct {
    int num;
    struct OSSL_CMP_ITAV **data;
} STACK_OF_OSSL_CMP_ITAV;

static int sk_OSSL_CMP_ITAV_num(const STACK_OF_OSSL_CMP_ITAV *s) {
    return s ? s->num : 0;
}

static struct OSSL_CMP_ITAV *sk_OSSL_CMP_ITAV_value(const STACK_OF_OSSL_CMP_ITAV *s, int i) {
    if (!s || i < 0 || i >= s->num) return NULL;
    return s->data[i];
}

/* ITAV carrying type and a stack of UTF8STRINGs as certProfile */
typedef struct OSSL_CMP_ITAV {
    ASN1_OBJECT *type;
    STACK_OF_ASN1_UTF8STRING *strs; /* for certProfile */
} OSSL_CMP_ITAV;

static ASN1_OBJECT *OSSL_CMP_ITAV_get0_type(const OSSL_CMP_ITAV *itav) {
    return itav ? itav->type : NULL;
}

static int OSSL_CMP_ITAV_get0_certProfile(const OSSL_CMP_ITAV *itav, STACK_OF_ASN1_UTF8STRING **out) {
    if (!itav || !out) return 0;
    *out = itav->strs;
    return 1;
}

/* CMP header holding generalInfo ITAVs */
typedef struct {
    STACK_OF_OSSL_CMP_ITAV *itavs;
} OSSL_CMP_HDR;

static STACK_OF_OSSL_CMP_ITAV *OSSL_CMP_HDR_get0_geninfo_ITAVs(const OSSL_CMP_HDR *hdr) {
    return hdr ? hdr->itavs : NULL;
}

/* CMP message with bodytype and header */
typedef struct {
    int bodytype;
    OSSL_CMP_HDR *hdr;
} OSSL_CMP_MSG;

static int OSSL_CMP_MSG_get_bodytype(const OSSL_CMP_MSG *msg) {
    return msg ? msg->bodytype : -1;
}

static const OSSL_CMP_HDR *OSSL_CMP_MSG_get0_header(const OSSL_CMP_MSG *msg) {
    return msg ? msg->hdr : NULL;
}

/* Minimal server context with only fields touched before the vulnerable strcmp */
typedef struct {
    int pollCount;
    int curr_pollCount;
} MY_SRV_CTX;

/* Dummy error raiser to keep code readable */
static void ERR_raise(int lib, int reason) {
    (void)lib; (void)reason;
}

/* Vulnerable function mirroring the relevant logic of apps/lib/cmp_mock_srv.c */
static void *process_cert_request(MY_SRV_CTX *ctx, const OSSL_CMP_MSG *cert_req,
                                  void **certOut, void **chainOut, void **caPubs) {
    (void)ctx; /* Unused beyond the pre-checks in this minimal reproducer */
    if (certOut) *certOut = NULL;
    if (chainOut) *chainOut = NULL;
    if (caPubs) *caPubs = NULL;

    /* accept cert profile for cr messages only with the configured name */
    if (OSSL_CMP_MSG_get_bodytype(cert_req) == OSSL_CMP_CR) {
        const OSSL_CMP_HDR *hdr = OSSL_CMP_MSG_get0_header(cert_req);
        STACK_OF_OSSL_CMP_ITAV *itavs = (STACK_OF_OSSL_CMP_ITAV *)OSSL_CMP_HDR_get0_geninfo_ITAVs(hdr);
        int i;

        for (i = 0; i < sk_OSSL_CMP_ITAV_num(itavs); i++) {
            OSSL_CMP_ITAV *itav = sk_OSSL_CMP_ITAV_value(itavs, i);
            ASN1_OBJECT *obj = OSSL_CMP_ITAV_get0_type(itav);
            STACK_OF_ASN1_UTF8STRING *strs;
            ASN1_UTF8STRING *str;
            const char *data;

            if (OBJ_obj2nid(obj) == NID_id_it_certProfile) {
                if (!OSSL_CMP_ITAV_get0_certProfile(itav, &strs))
                    return NULL;
                if (sk_ASN1_UTF8STRING_num(strs) < 1) {
                    ERR_raise(0, 0);
                    return NULL;
                }
                str = sk_ASN1_UTF8STRING_value(strs, 0);
                if (str == NULL || (data = (const char *)ASN1_STRING_get0_data(str)) == NULL) {
                    ERR_raise(0, 0);
                    return NULL;
                }
                /* Vulnerable use: ASN1_STRING_get0_data may not be NUL-terminated. */
                if (strcmp(data, "profile1") != 0) { /* OOB read here if data isn't NUL-terminated */
                    ERR_raise(0, 0);
                    return NULL;
                }
                break;
            }
        }
    }
    return NULL;
}

/* Helper to create a non-NUL-terminated ASN1_UTF8STRING with exact length */
static ASN1_UTF8STRING *make_asn1_utf8_exact(const char *s) {
    size_t len = strlen(s);
    ASN1_UTF8STRING *str = (ASN1_UTF8STRING *)calloc(1, sizeof(*str));
    if (!str) return NULL;
    str->data = (unsigned char *)malloc(len); /* allocate EXACT length, no room for NUL */
    if (!str->data) { free(str); return NULL; }
    memcpy(str->data, s, len); /* copy without trailing NUL */
    str->length = (int)len;
    return str;
}

int main(void) {
    /* Build certProfile ITAV with a UTF8String "profile1" that is NOT NUL-terminated */
    ASN1_UTF8STRING *utf8 = make_asn1_utf8_exact("profile1");
    if (!utf8) {
        fprintf(stderr, "alloc failed\n");
        return 1;
    }

    STACK_OF_ASN1_UTF8STRING *strs = (STACK_OF_ASN1_UTF8STRING *)calloc(1, sizeof(*strs));
    strs->num = 1;
    strs->data = (ASN1_UTF8STRING **)calloc(1, sizeof(ASN1_UTF8STRING *));
    strs->data[0] = utf8;

    ASN1_OBJECT *obj = (ASN1_OBJECT *)calloc(1, sizeof(*obj));
    obj->nid = NID_id_it_certProfile;

    OSSL_CMP_ITAV *itav = (OSSL_CMP_ITAV *)calloc(1, sizeof(*itav));
    itav->type = obj;
    itav->strs = strs;

    STACK_OF_OSSL_CMP_ITAV *itavs = (STACK_OF_OSSL_CMP_ITAV *)calloc(1, sizeof(*itavs));
    itavs->num = 1;
    itavs->data = (OSSL_CMP_ITAV **)calloc(1, sizeof(OSSL_CMP_ITAV *));
    itavs->data[0] = itav;

    OSSL_CMP_HDR *hdr = (OSSL_CMP_HDR *)calloc(1, sizeof(*hdr));
    hdr->itavs = itavs;

    OSSL_CMP_MSG msg;
    msg.bodytype = OSSL_CMP_CR; /* Ensure the CR path is taken */
    msg.hdr = hdr;

    MY_SRV_CTX ctx;
    ctx.pollCount = 0; /* Bypass polling logic */
    ctx.curr_pollCount = 0;

    void *certOut = NULL, *chainOut = NULL, *caPubs = NULL;
    /* Call into the vulnerable function: this will call strcmp on a non-NUL string */
    (void)process_cert_request(&ctx, &msg, &certOut, &chainOut, &caPubs);

    /* Clean up (not strictly necessary for ASan reproducer) */
    free(utf8->data);
    free(utf8);
    free(strs->data);
    free(strs);
    free(itavs->data);
    free(itavs);
    free(itav);
    free(obj);
    free(hdr);

    return 0;
}