#include <stdio.h>
#include <stdlib.h>

/* Minimal stubs and type re-declarations to emulate the vulnerable path
 * from python/libxml.c: libxml_prev()
 */

/* Avoid needing real Python or libxml2 headers */
typedef struct _PyObject {
    int dummy;
} PyObject;

/* Minimal xmlNode/Attr types */
typedef struct _xmlNode xmlNode;
struct _xmlNode {
    int type;
    xmlNode *next;
    xmlNode *prev;
};

typedef struct _xmlAttr {
    int type;
    xmlNode *next;
    xmlNode *prev;
} xmlAttr;

typedef xmlNode *xmlNodePtr;
typedef xmlAttr *xmlAttrPtr;

typedef struct _xmlNs {
    int dummy;
    struct _xmlNs *next;
} xmlNs;

typedef xmlNs *xmlNsPtr;

/* Constants (values don't matter for the crash, but needed to compile) */
#define XML_DOCUMENT_NODE        9
#define XML_HTML_DOCUMENT_NODE   13
#define XML_ATTRIBUTE_NODE       2
#define XML_NAMESPACE_DECL       18

/* ATTRIBUTE_UNUSED shim */
#define ATTRIBUTE_UNUSED

/* Stubs that mimic the Python C-API/libxml2-Python glue behavior */
static int PyArg_ParseTuple(PyObject *args, const char *fmt, PyObject **obj) {
    /* Pretend parsing succeeds and return some PyObject for libxml_prev */
    (void)args;
    (void)fmt;
    static PyObject dummy_obj;
    *obj = &dummy_obj;
    return 1; /* success */
}

static xmlNodePtr PyxmlNode_Get(PyObject *obj) {
    /* For an invalid input object, the real function may return NULL.
     * We emulate that here to trigger the bug.
     */
    (void)obj;
    return NULL; /* This is the crux: cause cur to be NULL */
}

static PyObject *libxml_xmlNodePtrWrap(xmlNodePtr res) {
    /* Dummy wrapper that would normally convert an xmlNodePtr to a PyObject */
    (void)res;
    static PyObject wrapped;
    return &wrapped;
}

/* Vulnerable function reimplemented from python/libxml.c (lines ~2346-2376) */
static PyObject *
libxml_prev(ATTRIBUTE_UNUSED PyObject * self, PyObject * args)
{
    PyObject *resultobj, *obj;
    xmlNodePtr cur;
    xmlNodePtr res;

    if (!PyArg_ParseTuple(args, "O:prev", &obj))
        return NULL;
    cur = PyxmlNode_Get(obj);

    /* Vulnerability: cur is dereferenced without a NULL check */
    switch (cur->type) {
        case XML_DOCUMENT_NODE:
        case XML_HTML_DOCUMENT_NODE:
            res = NULL;
            break;
        case XML_ATTRIBUTE_NODE: {
                xmlAttrPtr attr = (xmlAttrPtr) cur;
                res = (xmlNodePtr) attr->prev;
            }
            break;
        case XML_NAMESPACE_DECL:
            res = NULL;
            break;
        default:
            res = cur->prev;
            break;
    }
    resultobj = libxml_xmlNodePtrWrap(res);
    return resultobj;
}

int main(void) {
    /* Call libxml_prev with any dummy PyObject for args.
     * Our stubs ensure PyxmlNode_Get returns NULL so that
     * libxml_prev dereferences a NULL pointer (cur->type).
     */
    PyObject *dummy_args = NULL; /* Unused by our stub parser */

    printf("About to trigger NULL pointer dereference in libxml_prev()...\n");
    /* This call will crash with ASan due to switch(cur->type) when cur == NULL */
    (void)libxml_prev(NULL, dummy_args);

    /* We should never reach here */
    printf("If you see this, the crash did not occur as expected.\n");
    return 0;
}
