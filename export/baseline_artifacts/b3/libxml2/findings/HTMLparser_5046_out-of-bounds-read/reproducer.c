#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* Minimal stand-in types mimicking libxml2 structures used by htmlParseTryOrFinish */
typedef struct _xmlParserInput {
    const unsigned char *base;
    const unsigned char *cur;
    const unsigned char *end;
} xmlParserInput, *xmlParserInputPtr;

typedef struct _htmlParserCtxt {
    int instate;
    int endCheckState;
    int checkIndex;
    xmlParserInputPtr in;
} htmlParserCtxt, *htmlParserCtxtPtr;

/* Macros used by the vulnerable function */
#define SKIP(n) do { ctxt->in->cur += (n); } while (0)
#define UPP(n)  (toupper(ctxt->in->cur[(n)]))

/* Stubs for other parser helpers referenced in the original function */
static int htmlParseCharData(htmlParserCtxtPtr ctxt, int disable) {
    (void)ctxt; (void)disable; return 1; /* not used in this path */
}
static int htmlParseLookupCommentEnd(htmlParserCtxtPtr ctxt) {
    (void)ctxt; return -1; /* not used in this path */
}
static int htmlParseLookupString(htmlParserCtxtPtr ctxt, int start, const char *str, int len, int end) {
    (void)ctxt; (void)start; (void)str; (void)len; (void)end; return -1; /* not used in this path */
}
static void htmlParseComment(htmlParserCtxtPtr ctxt, int bogus) {
    (void)ctxt; (void)bogus;
}
static void htmlParseDocTypeDecl(htmlParserCtxtPtr ctxt) {
    (void)ctxt;
}

/*
 * Minimal reproduction of the vulnerable portion of htmlParseTryOrFinish
 * (mirrors the logic around lines 5029-5054 from the provided source context).
 */
static void htmlParseTryOrFinish(htmlParserCtxtPtr ctxt, int terminate) {
    xmlParserInputPtr in = ctxt->in;
    int avail = (int)(in->end - in->cur);
    if (avail < 1)
        return;

    int mode = ctxt->endCheckState;

    if (mode != 0) {
        if (htmlParseCharData(ctxt, !terminate) == 0)
            return;
    } else if (in->cur[0] == '<') {
        int next;

        if (avail < 2) {
            if (!terminate)
                return;
            next = ' ';
        } else {
            next = in->cur[1];
        }

        if (next == '!') {
            /* Bug: When terminate is true, the original code doesn't ensure avail >= 4 */
            if ((!terminate) && (avail < 4))
                return;
            /* Out-of-bounds read happens here when avail is 2 or 3 and terminate is true */
            if ((in->cur[2] == '-') && (in->cur[3] == '-')) {
                if ((!terminate) && (htmlParseLookupCommentEnd(ctxt) < 0))
                    return;
                SKIP(4);
                htmlParseComment(ctxt, 0);
            }
            /* Stop here; we only need to trigger the OOB read above. */
            return;
        }
    }
}

int main(void) {
    /* Allocate a minimal input buffer of exactly 2 bytes: "<!" */
    unsigned char *buf = (unsigned char *)malloc(2);
    if (!buf) return 1;
    memcpy(buf, "<!", 2);

    xmlParserInput *in = (xmlParserInput *)calloc(1, sizeof(*in));
    htmlParserCtxt *ctxt = (htmlParserCtxt *)calloc(1, sizeof(*ctxt));
    if (!in || !ctxt) return 1;

    in->base = buf;
    in->cur  = buf;
    in->end  = buf + 2; /* avail == 2 */

    ctxt->in = in;
    ctxt->instate = 0;       /* not relevant for this minimal path */
    ctxt->endCheckState = 0; /* force the '<' handling path */

    /*
     * Call with terminate == 1 to emulate end-of-input condition that
     * triggers the buggy read of in->cur[2] and in->cur[3].
     */
    htmlParseTryOrFinish(ctxt, 1);

    /* Cleanup (unreached if ASan aborts on OOB) */
    free((void*)in);
    free((void*)ctxt);
    free(buf);

    return 0;
}
