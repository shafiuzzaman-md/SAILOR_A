#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stddef.h>

/* Minimal type and macro definitions to mimic libxml2 */
typedef unsigned char xmlChar;

typedef struct _xmlParserInput {
    const xmlChar *cur;
    const xmlChar *end;
} xmlParserInput, *xmlParserInputPtr;

typedef struct _xmlParserCtxt {
    xmlParserInputPtr input;
    size_t checkIndex;
    int endCheckState;
} xmlParserCtxt, *xmlParserCtxtPtr;

#define IS_BLANK_CH(c) (((c) == 0x20) || ((c) == 0x9) || ((c) == 0xA) || ((c) == 0xD))

/* Vulnerable function (reduced to the relevant parts) */
static int xmlParseLookupInternalSubset(xmlParserCtxtPtr ctxt) {
    const xmlChar *cur, *start;
    const xmlChar *end = ctxt->input->end;
    int state = ctxt->endCheckState;
    size_t index; /* Unused here, but present in original context */

    if (ctxt->checkIndex == 0) {
        cur = ctxt->input->cur + 1;
    } else {
        cur = ctxt->input->cur + ctxt->checkIndex;
    }
    start = cur;

    while (cur < end) {
        if (state == '-') {
            /* OOB read when cur == end-1 or end-2 (accesses cur[1] / cur[2]) */
            if ((*cur == '-') &&
                (cur[1] == '-') &&
                (cur[2] == '>')) {
                state = 0;
                cur += 3;
                start = cur;
                continue;
            }
        }
        else if (state == ']') {
            if (*cur == '>') {
                ctxt->checkIndex = 0;
                ctxt->endCheckState = 0;
                return(1);
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
                return(1);
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
            /* Rest of original parser not needed for the reproducer */
        }
        cur++;
    }

    /* Update context as original code would do on incomplete data */
    ctxt->checkIndex = (size_t)(start - ctxt->input->cur);
    ctxt->endCheckState = state;
    return 0;
}

int main(void) {
    /* Allocate a buffer of exactly 4 bytes so that reading buf[4] is OOB */
    xmlChar *buf = (xmlChar *)malloc(4);
    if (!buf) {
        perror("malloc");
        return 1;
    }

    /* Fill buffer; place '-' at the last valid position (index 3) */
    memset(buf, 'A', 4);
    buf[3] = '-';

    /* Set up a minimal parser input/context so that:
     *  - checkIndex == 0 -> cur = input.cur + 1
     *  - input.cur points to buf + 2, so cur becomes buf + 3 (last valid byte)
     *  - end points to one-past-the-end (buf + 4)
     *  - state is '-' to take the comment-closing path and read cur[1]
     */
    xmlParserInput input;
    input.cur = buf + 2;      /* cur = input.cur + 1 => buf + 3 */
    input.end = buf + 4;      /* one past the allocated buffer */

    xmlParserCtxt ctxt;
    ctxt.input = &input;
    ctxt.checkIndex = 0;      /* forces cur = input.cur + 1 */
    ctxt.endCheckState = '-'; /* choose the branch which checks for "-->" */

    /* This call will read cur[1] when cur == end - 1, causing OOB read */
    int ret = xmlParseLookupInternalSubset(&ctxt);

    /* Prevent optimizing away and show function returned */
    printf("xmlParseLookupInternalSubset returned %d\n", ret);

    free(buf);
    return 0;
}
