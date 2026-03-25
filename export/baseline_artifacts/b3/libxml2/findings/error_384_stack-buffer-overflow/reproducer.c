#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

/* Minimal typedefs to mirror libxml2 */
typedef unsigned char xmlChar;

typedef struct _xmlParserInput {
    /* Only the fields used by this reproducer are declared */
    const xmlChar *cur;  /* Checked for NULL in the vulnerable function */
    int line;            /* Unused here but present in real struct */
} xmlParserInput, *xmlParserInputPtr;

/* Prototype to mirror libxml2's error callback type */
typedef void (*xmlGenericErrorFunc)(void *ctx, const char *msg, ...);

/* Stub for xmlParserInputGetWindow to force col == 80 and 80 bytes of content.
 * This interposes the real function (if linked), but our reproducer is self-contained.
 */
void xmlParserInputGetWindow(xmlParserInputPtr input, const xmlChar **base, int *len, int *col) {
    static xmlChar buf[80];
    /* Fill with non-tab printable characters so the space-replacement loop runs fully */
    for (int i = 0; i < 80; i++)
        buf[i] = (xmlChar)('A');

    if (base) *base = buf;
    if (len)  *len  = 80; /* Maximum window size copied by the vulnerable code */
    if (col)  *col  = 80; /* Caret position at the very end of the 80-char window */

    (void)input; /* Unused in this stub */
}

/* Simple printf-like error channel used by the vulnerable function */
static void myErrorChannel(void *data, const char *msg, ...) {
    (void)data;
    va_list ap;
    va_start(ap, msg);
    vfprintf(stdout, msg, ap);
    va_end(ap);
}

/* Copy of the vulnerable internal function (from error.c).
 * The bug: when col == 80, content[n++] = '^' writes to index 80 (ok for size 81),
 * but then content[n] = 0 writes index 81, which is one past the end.
 */
static void xmlParserPrintFileContextInternal(xmlParserInputPtr input,
                                              xmlGenericErrorFunc channel,
                                              void *data) {
    const xmlChar *start;
    int n, col;
    xmlChar content[81]; /* space for 80 chars + line terminator */

    if ((input == NULL) || (input->cur == NULL))
        return;

    n = (int)sizeof(content) - 1; /* 80 */
    xmlParserInputGetWindow(input, &start, &n, &col);

    memcpy(content, start, (size_t)n);
    content[n] = 0;
    /* print out the selected text */
    channel(data, "%s\n", (const char *)content);
    /* create blank line with problem pointer */
    for (n = 0; n < col; n++) {
        if (content[n] != '\t')
            content[n] = ' ';
    }
    content[n++] = '^';
    content[n] = 0; /* When col == 80, this writes one byte past the end (index 81) */
    channel(data, "%s\n", (const char *)content);
}

/* Public wrapper mirroring libxml2's API */
void xmlParserPrintFileContext(struct _xmlParserInput *input) {
    xmlParserPrintFileContextInternal(input, myErrorChannel, NULL);
    (void)input;
}

int main(void) {
    /* Prepare a fake parser input with non-NULL cur so the function doesn't early-return */
    static xmlChar dummy = (xmlChar)'X';
    xmlParserInput in;
    memset(&in, 0, sizeof(in));
    in.cur = &dummy; /* Non-NULL triggers the vulnerable path */

    /* Call the public wrapper which invokes the vulnerable internal function */
    xmlParserPrintFileContext(&in);

    return 0;
}
