#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Minimal typedefs/macros to mimic libxml2 environment */
typedef unsigned char xmlChar;
#define ATTRIBUTE_UNUSED
#define BAD_CAST (xmlChar *)
#define IS_BLANK_CH(c) ((c) == 0x20 || (c) == 0x9 || (c) == 0xA || (c) == 0xD)

/* Minimal validation context carrying just the member used by the function */
typedef struct _xmlRelaxNGValidCtxt {
    void *elem; /* execution context pointer checked for non-NULL */
} xmlRelaxNGValidCtxt;

/* Stub for internal function called later in the function (won't be reached) */
int xmlRegExecPushString(void *exec, const xmlChar *value, void *data) {
    (void)exec; (void)value; (void)data;
    return 0; /* success */
}

/* Vulnerable function reproduced from the source context */
int xmlRelaxNGValidatePushCData(xmlRelaxNGValidCtxt *ctxt,
                                const xmlChar * data, int len ATTRIBUTE_UNUSED)
{
    int ret = 1;

    if ((ctxt == NULL) || (ctxt->elem == NULL) || (data == NULL))
        return (-1);

    /* BUG: ignores len and scans until NUL, potentially reading past buffer */
    while (*data != 0) {
        if (!IS_BLANK_CH(*data))
            break;
        data++;
    }
    if (*data == 0)
        return (1);

    ret = xmlRegExecPushString(ctxt->elem, BAD_CAST "#text", ctxt);
    if (ret < 0) {
        return (-1);
    }
    return (1);
}

int main(void) {
    /* Allocate a small buffer of blanks without a terminating NUL */
    size_t len = 8;
    xmlChar *buf = (xmlChar *)malloc(len);
    if (!buf) {
        perror("malloc");
        return 1;
    }
    /* Fill with spaces so IS_BLANK_CH is true for all bytes */
    memset(buf, 0x20, len); /* ' ' */

    /* Prepare a context with non-NULL elem to pass initial checks */
    xmlRelaxNGValidCtxt ctxt;
    ctxt.elem = &ctxt; /* any non-NULL pointer */

    /* This call will read past the end of buf because there's no NUL terminator
       and the function ignores the provided length. With ASan, this triggers
       an out-of-bounds read report. */
    (void)xmlRelaxNGValidatePushCData(&ctxt, buf, (int)len);

    /* If the bug didn't trigger (unexpected), free and exit */
    free(buf);
    return 0;
}
