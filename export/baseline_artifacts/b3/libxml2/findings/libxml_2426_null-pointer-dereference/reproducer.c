#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

/* Minimal stubs to mimic the parts of Python and libxml2 used by the vulnerable code */

#ifndef ATTRIBUTE_UNUSED
#define ATTRIBUTE_UNUSED
#endif

/* Minimal Python C-API stubs */
typedef struct _PyObject { int dummy; } PyObject;

/* Simulate PyArg_ParseTuple(args, "O:last", &obj) */
int PyArg_ParseTuple(PyObject *args, const char *fmt, ...) {
    (void)args; (void)fmt;
    va_list ap;
    va_start(ap, fmt);
    PyObject **out = va_arg(ap, PyObject**);
    static PyObject bad_obj; /* some non-NULL PyObject */
    *out = &bad_obj;
    va_end(ap);
    return 1; /* success */
}

/* Minimal libxml2 node types and pointers */
enum xmlElementType {
    XML_ELEMENT_NODE = 1,
    XML_ATTRIBUTE_NODE = 2,
    XML_ENTITY_REF_NODE = 5,
    XML_ENTITY_NODE = 6,
    XML_PI_NODE = 7,
    XML_COMMENT_NODE = 8,
    XML_DOCUMENT_NODE = 9,
    XML_HTML_DOCUMENT_NODE = 10,
    XML_DTD_NODE = 11
};

typedef struct _xmlNode xmlNode;
struct _xmlNode {
    int type;
    struct _xmlNode *children;
    struct _xmlNode *last;
};

typedef xmlNode *xmlNodePtr;

typedef struct _xmlAttr {
    int type;
    struct _xmlNode *children;
    struct _xmlNode *last;
} *xmlAttrPtr;

/* Stub: returns NULL to simulate a bad Python object that cannot be converted */
xmlNodePtr PyxmlNode_Get(PyObject *obj) {
    (void)obj;
    return NULL; /* This triggers the NULL dereference in libxml_last */
}

/* Stub: never reached due to crash, but needed for linking */
PyObject *libxml_xmlNodePtrWrap(xmlNodePtr p) {
    static PyObject dummy;
    (void)p;
    return &dummy;
}

/* Vulnerable function copied/adapted from python/libxml.c */
static PyObject *
libxml_last(ATTRIBUTE_UNUSED PyObject * self, PyObject * args)
{
    PyObject *resultobj, *obj;
    xmlNodePtr cur;
    xmlNodePtr res;

    if (!PyArg_ParseTuple(args, "O:last", &obj))
        return NULL;
    cur = PyxmlNode_Get(obj);

    /* Vulnerable NULL dereference: cur is NULL, yet cur->type is accessed */
    switch (cur->type) {
        case XML_ELEMENT_NODE:
        case XML_ENTITY_REF_NODE:
        case XML_ENTITY_NODE:
        case XML_PI_NODE:
        case XML_COMMENT_NODE:
        case XML_DOCUMENT_NODE:
        case XML_HTML_DOCUMENT_NODE:
        case XML_DTD_NODE:
            res = cur->last;
            break;
        case XML_ATTRIBUTE_NODE: {
                xmlAttrPtr attr = (xmlAttrPtr) cur;
                res = attr->last;
                break;
            }
        default:
            res = NULL;
            break;
    }
    resultobj = libxml_xmlNodePtrWrap(res);
    return resultobj;
}

int main(void) {
    /* Call the vulnerable function with a dummy args object. Our PyArg_ParseTuple
       stub will extract &obj and set it to a non-NULL PyObject*, then PyxmlNode_Get
       returns NULL, leading to switch(cur->type) on a NULL pointer. */
    PyObject args;
    (void)libxml_last(NULL, &args);

    /* If the process didn't crash (it should), print something. */
    puts("Unexpectedly survived null dereference");
    return 0;
}
