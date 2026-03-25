#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Minimal re-declarations of types/macros used in the vulnerable code path
#define STACK_OF(TYPE) struct stack_##TYPE

typedef struct mock_srv_ctx {
    int sendError;
} mock_srv_ctx;

typedef struct {
    mock_srv_ctx *custom_ctx;
} OSSL_CMP_SRV_CTX;

static mock_srv_ctx *OSSL_CMP_SRV_CTX_get0_custom_ctx(OSSL_CMP_SRV_CTX *srv_ctx) {
    return srv_ctx ? srv_ctx->custom_ctx : NULL;
}

typedef struct {
    int bodytype;
} OSSL_CMP_MSG;

static int OSSL_CMP_MSG_get_bodytype(const OSSL_CMP_MSG *msg) {
    return msg ? msg->bodytype : 0;
}

typedef struct asn1_object_st {
    int nid;
} ASN1_OBJECT;

static int OBJ_obj2nid(const ASN1_OBJECT *obj) {
    return obj ? obj->nid : 0;
}

// Dummy ITAV object
typedef struct itav_st {
    int dummy;
    ASN1_OBJECT obj;
} OSSL_CMP_ITAV;

static ASN1_OBJECT *OSSL_CMP_ITAV_get0_type(OSSL_CMP_ITAV *itav) {
    return itav ? &itav->obj : NULL;
}

// Minimal stack implementation for OSSL_CMP_ITAV
struct stack_OSSL_CMP_ITAV {
    size_t num;
    size_t cap;
    OSSL_CMP_ITAV **data;
};

typedef STACK_OF(OSSL_CMP_ITAV) ITAV_STACK;

static STACK_OF(OSSL_CMP_ITAV) *sk_OSSL_CMP_ITAV_new_reserve(void *cmp, int n) {
    (void)cmp;
    if (n < 0) return NULL;
    ITAV_STACK *st = (ITAV_STACK *)malloc(sizeof(*st));
    if (!st) return NULL;
    st->num = 0;
    st->cap = (size_t)n;
    st->data = (OSSL_CMP_ITAV **)calloc((size_t)n > 0 ? (size_t)n : 1, sizeof(OSSL_CMP_ITAV *));
    if (!st->data) {
        free(st);
        return NULL;
    }
    return st;
}

static int sk_OSSL_CMP_ITAV_push(STACK_OF(OSSL_CMP_ITAV) *st, OSSL_CMP_ITAV *val) {
    if (!st) return 0;
    if (st->num == st->cap) {
        size_t newcap = st->cap ? st->cap * 2 : 1;
        OSSL_CMP_ITAV **nd = (OSSL_CMP_ITAV **)realloc(st->data, newcap * sizeof(*nd));
        if (!nd) return 0;
        st->data = nd;
        st->cap = newcap;
    }
    st->data[st->num++] = val;
    return 1;
}

static int sk_OSSL_CMP_ITAV_num(const STACK_OF(OSSL_CMP_ITAV) *st) {
    return st ? (int)st->num : 0;
}

static OSSL_CMP_ITAV *sk_OSSL_CMP_ITAV_value(const STACK_OF(OSSL_CMP_ITAV) *st, int idx) {
    if (!st || idx < 0 || (size_t)idx >= st->num) return NULL;
    return st->data[idx];
}

static void sk_OSSL_CMP_ITAV_free(STACK_OF(OSSL_CMP_ITAV) *st) {
    if (!st) return;
    // Free only the container, not the elements (matches how out is used in the snippet)
    free(st->data);
    free(st);
}

// Stubs to satisfy references in the original code
#define ERR_LIB_CMP 0
#define CMP_R_NULL_ARGUMENT 1
#define CMP_R_ERROR_PROCESSING_MESSAGE 2
static void ERR_raise(int lib, int reason) {
    (void)lib; (void)reason; // no-op stub
}

// process_genm_itav will simulate a failure by returning NULL to hit the error path
static OSSL_CMP_ITAV *process_genm_itav(mock_srv_ctx *ctx, int nid, OSSL_CMP_ITAV *req) {
    (void)ctx; (void)nid; (void)req;
    return NULL; // Force the rsp == NULL path
}

// Vulnerable function replicated from the source context (reduced to essentials)
static int process_genm(OSSL_CMP_SRV_CTX *srv_ctx,
                        const OSSL_CMP_MSG *genm,
                        const STACK_OF(OSSL_CMP_ITAV) *in,
                        STACK_OF(OSSL_CMP_ITAV) **out)
{
    mock_srv_ctx *ctx = OSSL_CMP_SRV_CTX_get0_custom_ctx(srv_ctx);

    if (ctx == NULL || genm == NULL || in == NULL || out == NULL) {
        ERR_raise(ERR_LIB_CMP, CMP_R_NULL_ARGUMENT);
        return 0;
    }
    if (ctx->sendError == 1
        || ctx->sendError == OSSL_CMP_MSG_get_bodytype(genm)
        || sk_OSSL_CMP_ITAV_num(in) > 1) {
        ERR_raise(ERR_LIB_CMP, CMP_R_ERROR_PROCESSING_MESSAGE);
        return 0;
    }
    if (sk_OSSL_CMP_ITAV_num(in) == 1) {
        OSSL_CMP_ITAV *req = sk_OSSL_CMP_ITAV_value(in, 0), *rsp;
        ASN1_OBJECT *obj = OSSL_CMP_ITAV_get0_type(req);

        if ((*out = sk_OSSL_CMP_ITAV_new_reserve(NULL, 1)) == NULL)
            return 0;
        rsp = process_genm_itav(ctx, OBJ_obj2nid(obj), req);
        if (rsp != NULL && sk_OSSL_CMP_ITAV_push(*out, rsp))
            return 1;
        // Bug: frees *out but does not set *out = NULL
        sk_OSSL_CMP_ITAV_free(*out);
        return 0;
    }

    // Not taken in this reproducer
    // *out = sk_OSSL_CMP_ITAV_deep_copy(in, OSSL_CMP_ITAV_dup, OSSL_CMP_ITAV_free);
    // return *out != NULL;
    return 0;
}

int main(void) {
    // Set up server and context
    mock_srv_ctx mctx;
    mctx.sendError = 0; // ensure we don't bail out early

    OSSL_CMP_SRV_CTX srv;
    srv.custom_ctx = &mctx;

    // Prepare a genm message with arbitrary bodytype not equal to sendError
    OSSL_CMP_MSG genm;
    genm.bodytype = 42;

    // Prepare input stack with exactly one ITAV to take the single-ITAV branch
    STACK_OF(OSSL_CMP_ITAV) *in = sk_OSSL_CMP_ITAV_new_reserve(NULL, 1);
    if (!in) {
        fprintf(stderr, "Failed to allocate input stack\n");
        return 1;
    }
    OSSL_CMP_ITAV *one = (OSSL_CMP_ITAV *)malloc(sizeof(*one));
    if (!one) {
        fprintf(stderr, "Failed to allocate ITAV\n");
        return 1;
    }
    memset(one, 0, sizeof(*one));
    one->obj.nid = 1234; // arbitrary
    if (!sk_OSSL_CMP_ITAV_push(in, one)) {
        fprintf(stderr, "Failed to push ITAV\n");
        return 1;
    }

    // Output pointer as expected by process_genm
    STACK_OF(OSSL_CMP_ITAV) *out = NULL;

    // Call the vulnerable function; it will allocate *out, then free it on error and return 0, leaving a dangling pointer in out
    int rc = process_genm(&srv, &genm, in, &out);
    printf("process_genm returned: %d\n", rc);

    // Typical caller cleanup on failure: free(out) if non-NULL. This uses the dangling pointer, causing use-after-free / double free.
    // ASan should report a heap-use-after-free or double-free here.
    if (out != NULL) {
        printf("Freeing out after failure (should trigger ASan due to UAF/double free)\n");
        sk_OSSL_CMP_ITAV_free(out);
    }

    // Clean up input stack and elements
    if (in) {
        // Free the single element added to 'in'
        if (sk_OSSL_CMP_ITAV_num(in) == 1) {
            OSSL_CMP_ITAV *elem = sk_OSSL_CMP_ITAV_value(in, 0);
            free(elem);
        }
        sk_OSSL_CMP_ITAV_free(in);
    }

    return 0;
}
