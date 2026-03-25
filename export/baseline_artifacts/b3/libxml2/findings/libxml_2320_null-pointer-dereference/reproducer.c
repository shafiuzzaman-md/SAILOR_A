#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

/* Minimal stubs and type re-definitions to reproduce the bug path */

#define ATTRIBUTE_UNUSED

/* Fake Python object */
typedef struct _PyObject {
    void *data;
} PyObject;

/* Minimal XML node/type stubs */
typedef struct _xmlNode xmlNode;
typedef struct _xmlAttr xmlAttr;
typedef struct _xmlNs xmlNs;

typedef xmlNode *xmlNodePtr;
typedef xmlAttr *xmlAttrPtr;
typedef xmlNs *xmlNsPtr;

enum {
    XML_ELEMENT_NODE = 1,
    XML_ATTRIBUTE_NODE = 2,
    XML_TEXT_NODE = 3,
    XML_DOCUMENT_NODE = 9,
    XML_HTML_DOCUMENT_NODE = 13,
    XML_NAMESPACE_DECL = 18
};

struct _xmlAttr {
    int dummy;
    xmlAttrPtr next;
};

struct _xmlNs {
    int dummy;
    xmlNsPtr next;
};

struct _xmlNode {
    int type;
    xmlNodePtr next;
    void *doc;
    xmlAttrPtr properties;
};

/* Stub of Python's PyArg_ParseTuple: grab first PyObject* from a fake tuple */
int PyArg_ParseTuple(PyObject *args, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    PyObject **out = va_arg(ap, PyObject **);
    va_end(ap);
    if (!out) return 0;
    /* Our fake tuple stores the single object in args->data */
    *out = (PyObject *)args->data;
    return 1; /* pretend parse success */
}

/* Stub: returns NULL to simulate invalid input to PyxmlNode_Get */
xmlNodePtr PyxmlNode_Get(PyObject *obj) {
    (void)obj;
    return NULL; /* critical for triggering the NULL deref */
}

/* Stub wrappers returning dummy Python objects */
PyObject *libxml_xmlNodePtrWrap(xmlNodePtr res) {
    PyObject *o = (PyObject *)malloc(sizeof(PyObject));
    o->data = res;
    return o;
}

/* Vulnerable function copied/adapted from python/libxml.c */
PyObject *libxml_next(ATTRIBUTE_UNUSED PyObject *self, PyObject *args) {
    PyObject *resultobj, *obj;
    xmlNodePtr cur;
    xmlNodePtr res;

    if (!PyArg_ParseTuple(args, "O:next", &obj))
        return NULL;
    cur = PyxmlNode_Get(obj);

    /* BUG: cur may be NULL; dereferencing cur->type triggers a null-deref */
    switch (cur->type) {
        case XML_DOCUMENT_NODE:
        case XML_HTML_DOCUMENT_NODE:
            res = NULL;
            break;
        case XML_ATTRIBUTE_NODE: {
            xmlAttrPtr attr = (xmlAttrPtr)cur;
            res = (xmlNodePtr)attr->next;
            break;
        }
        case XML_NAMESPACE_DECL: {
            xmlNsPtr ns = (xmlNsPtr)cur;
            res = (xmlNodePtr)ns->next;
            break;
        }
        default:
            res = cur->next;
            break;
    }
    resultobj = libxml_xmlNodePtrWrap(res);
    return resultobj;
}

int main(void) {
    /* Prepare a fake Python argument tuple holding a single object */
    PyObject bad_obj = { .data = NULL };     /* any object; PyxmlNode_Get will return NULL */
    PyObject args = { .data = &bad_obj };    /* our PyArg_ParseTuple will pull this out */

    /* Call the vulnerable function. It will call PyxmlNode_Get(&bad_obj) -> NULL,
       then immediately dereference cur->type, crashing with ASan diagnostics. */
    PyObject *ret = libxml_next(NULL, &args);

    /* If it somehow returns, free the dummy result to avoid leaks in non-crash runs */
    free(ret);

    return 0;
}
