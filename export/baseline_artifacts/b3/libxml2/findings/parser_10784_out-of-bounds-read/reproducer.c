#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

/* Minimal type re-declarations to call the vulnerable function */
typedef struct _xmlParserInput {
    const unsigned char *cur;
    const unsigned char *end;
} xmlParserInput, *xmlParserInputPtr;

typedef struct _xmlParserCtxt {
    xmlParserInputPtr input;
    long checkIndex;
    int endCheckState;
} xmlParserCtxt, *xmlParserCtxtPtr;

/* Minimal IS_BLANK_CH macro since original code uses it in other branches */
#define IS_BLANK_CH(c) (((c) == 0x20) || ((c) == 0x9) || ((c) == 0xD) || ((c) == 0xA))

/*
 * Vulnerable function: xmlParseLookupInternalSubset
 * This is a minimized reproduction of the offending logic focusing on the
 * out-of-bounds read when checking for the start of a comment ("<!--").
 */
int xmlParseLookupInternalSubset(xmlParserCtxtPtr ctxt) {
    const unsigned char *cur, *end, *start;
    int state;

    if (ctxt == NULL || ctxt->input == NULL)
        return 0;

    /* Start scanning at current position + previously saved index */
    cur = ctxt->input->cur + (ctxt->checkIndex > 0 ? ctxt->checkIndex : 0);
    end = ctxt->input->end;
    start = cur;
    state = ctxt->endCheckState;

    /*
     * This loop mimics the structure shown in the source snippet. The bug is
     * in the branch checking for "<!--" using cur[1], cur[2], cur[3] while the
     * only loop guard is (cur < end).
     */
    while (cur < end) {
        if (state == '-') {
            /* Not needed for triggering the bug; keep minimal behavior. */
        }
        else if (state == ']') {
            if (*cur == '>') {
                ctxt->checkIndex = 0;
                ctxt->endCheckState = 0;
                return 1;
            }
            if (IS_BLANK_CH(*cur)) {
                state = ' ';
            } else if (*cur != ']') {
                state = 0;
                start = cur;
                continue;
            }
        }
        else if (state == ' ') {
            if (*cur == '>') {
                ctxt->checkIndex = 0;
                ctxt->endCheckState = 0;
                return 1;
            }
            if (!IS_BLANK_CH(*cur)) {
                state = 0;
                start = cur;
                continue;
            }
        }
        else if (state != 0) {
            if (*cur == state) {
                state = 0;
                start = cur + 1;
            }
        }
        else if (*cur == '<') {
            /*
             * Vulnerable lookahead: accesses cur[1], cur[2], cur[3] without
             * ensuring there are 4 bytes available up to 'end'. If cur is
             * near end (e.g., last byte), these reads go out of bounds.
             */
            if ((cur[1] == '!') &&    /* OOB if cur == end - 1 */
                (cur[2] == '-') &&    /* OOB if cur >= end - 2 */
                (cur[3] == '-')) {    /* OOB if cur >= end - 3 */
                state = '-';
                cur += 4;
                start = cur;
                continue;
            }
        }
        else if ((*cur == '"') || (*cur == '\'') || (*cur == ']')) {
            state = *cur;
        }

        cur++;
    }

    /* Tail handling kept minimal; not relevant to triggering the bug */
    if ((state == 0) || (state == '-')) {
        if (cur - start < 3)
            cur = start;
        else
            cur -= 3;
    }

    long index = (long)(cur - ctxt->input->cur);
    if (index > LONG_MAX) {
        ctxt->checkIndex = 0;
        ctxt->endCheckState = 0;
        return 1;
    }
    ctxt->checkIndex = index;
    ctxt->endCheckState = state;
    return 0;
}

int main(void) {
    /*
     * Craft input buffer where the last and only byte is '<'.
     * The loop condition (cur < end) is true once, and when it checks for
     * the start of a comment ("<!--"), it reads cur[1..3], which are out of
     * bounds. ASan should report a global-buffer-overflow.
     */
    static const unsigned char buf[1] = {'<'};  /* length 1, last byte is '<' */

    xmlParserInput input;
    input.cur = buf;
    input.end = buf + sizeof(buf);

    xmlParserCtxt ctxt;
    memset(&ctxt, 0, sizeof(ctxt));
    ctxt.input = &input;
    ctxt.checkIndex = 0;     /* start scanning at beginning */
    ctxt.endCheckState = 0;  /* neutral state */

    /* Call the vulnerable function: should trigger OOB read under ASan */
    int ret = xmlParseLookupInternalSubset(&ctxt);

    /* Prevent optimization-out and show that we returned. */
    printf("xmlParseLookupInternalSubset returned %d\n", ret);
    return 0;
}
