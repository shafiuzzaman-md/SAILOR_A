#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdint.h>

/*
 * Minimal stand-in data structures to mimic the relevant libxml2 HTML parser
 * state. We only implement the fields and logic needed to hit the buggy code
 * path in htmlParseTryOrFinish.
 */

typedef struct _xmlParserInput {
    const unsigned char *cur;
    const unsigned char *end;
} xmlParserInput;

typedef struct _htmlParserCtxt {
    xmlParserInput *input;
    int endCheckState;
    int instate;
    int disableSAX;
    void *sax;
    void *userData;
    int checkIndex;
} htmlParserCtxt;

/* UPP(i): uppercase of in->cur[i] like libxml2's UPP macro */
#define UPP(i) (toupper((unsigned char)in->cur[(i)]))

/*
 * Reimplementation of the vulnerable portion of htmlParseTryOrFinish
 * from HTMLparser.c around lines 5018..5098. This reproduces the
 * out-of-bounds read when terminate is true and fewer than 9 bytes are
 * available after "<!" while checking for "DOCTYPE" using UPP(2)..UPP(8).
 */
static void htmlParseTryOrFinish(htmlParserCtxt *ctxt, int terminate) {
    xmlParserInput *in = ctxt->input;
    int avail = (int)(in->end - in->cur);

    if (avail < 1)
        return;

    int mode = ctxt->endCheckState;

    if (mode != 0) {
        // Not relevant for this reproducer
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
            if ((!terminate) && (avail < 4))
                return;

            /* Comment check first (safe for our crafted input) */
            if ((in->cur[2] == '-') && (in->cur[3] == '-')) {
                // Would parse comment; not taken for our input
                return;
            }

            /* BUG: When terminate == true, there is no avail >= 9 check */
            if ((!terminate) && (avail < 9))
                return;

            /* This condition chain reads UPP(2)..UPP(8) even if avail < 9 */
            if ((UPP(2) == 'D') && (UPP(3) == 'O') &&
                (UPP(4) == 'C') && (UPP(5) == 'T') &&
                (UPP(6) == 'Y') && (UPP(7) == 'P') &&
                (UPP(8) == 'E')) {
                // Would parse DOCTYPE; not reached, but the read at UPP(8)
                // already triggered OOB with ASan.
            } else {
                // Would handle bogus declaration/comment path
            }
        }
    }
}

int main(void) {
    /*
     * Craft the minimal buffer: "<!DOCTYP" (8 bytes total), which makes
     * avail == 8 at the point of evaluation. The code then evaluates
     * (UPP(8) == 'E'), causing an out-of-bounds read at index 8.
     */
    static const unsigned char buf[8] = {
        '<','!','D','O','C','T','Y','P'
    };

    xmlParserInput in;
    in.cur = buf;
    in.end = buf + sizeof(buf); // avail = 8

    htmlParserCtxt ctxt;
    memset(&ctxt, 0, sizeof(ctxt));
    ctxt.input = &in;
    ctxt.endCheckState = 0; // force the '<' handling path

    /* terminate = 1 simulates end-of-input in push parsing,
     * which skips the avail >= 9 guard in the buggy code.
     */
    htmlParseTryOrFinish(&ctxt, 1);

    /* If AddressSanitizer is enabled, the above call should have
     * triggered an out-of-bounds read. */
    return 0;
}
