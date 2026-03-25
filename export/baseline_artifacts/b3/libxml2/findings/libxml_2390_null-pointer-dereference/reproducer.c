#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#ifndef ATTRIBUTE_UNUSED
#define ATTRIBUTE_UNUSED
#endif

/* Minimal stand-ins for libxml2 types used in the snippet */
typedef struct _xmlNode xmlNode;
typedef struct _xmlAttr xmlAttr;
typedef xmlNode* xmlNodePtr;
typedef xmlAttr* xmlAttrPtr;

enum xmlElementType {
    XML_ELEMENT_NODE = 1,
    XML_ATTRIBUTE_NODE = 2,
    XML_ENTITY_REF_NODE = 5,
    XML_ENTITY_NODE = 6,
    XML_PI_NODE = 7,
    XML_COMMENT_NODE = 8,
    XML_DOCUMENT_NODE = 9,
    XML_HTML_DOCUMENT_NODE = 13,
    XML_DTD_NODE = 17,
    XML_NAMESPACE_DECL = 18
};

struct _xmlNode {
    int type;
    xmlNodePtr children;
    xmlNodePtr prev;
};

struct _xmlAttr {
    int type;
    xmlNodePtr children;
    xmlAttrPtr prev;
};

/* Minimal stand-ins for CPython API used in the snippet */
typedef struct _PyObject {
    int is_valid; /* 0 => invalid object that makes PyxmlNode_Get return NULL */
} PyObject;

/* Stub: Always succeed and set obj to an invalid object so that PyxmlNode_Get returns NULL */
int PyArg_ParseTuple(PyObject *args, const char *format, ...) {
    (void)args; (void)format;
    va_list ap;
    va_start(ap, format);
    PyObject **obj_out = va_arg(ap, PyObject **);
    static PyObject invalid_obj;
    invalid_obj.is_valid = 0; /* force invalid */
    *obj_out = &invalid_obj;
    va_end(ap);
    return 1; /* success */
}

/* Stub: Return NULL for invalid inputs to emulate the real behavior */
xmlNodePtr PyxmlNode_Get(PyObject *obj) {
    if (obj == NULL) return NULL;
    if (obj->is_valid == 0) return NULL; /* emulate failure path */
    static xmlNode dummy = { .type = XML_ELEMENT_NODE, .children = NULL, .prev = NULL };
    return &dummy; /* not used in this reproducer */
}

/* Stub: Wraps an xmlNodePtr into a PyObject (not important for the crash) */
PyObject *libxml_xmlNodePtrWrap(xmlNodePtr res) {
    PyObject *o = (PyObject *)malloc(sizeof(PyObject));
    o->is_valid = (res != NULL);
    return o;
}

/* Vulnerable function copied/approximated from python/libxml.c */
PyObject *
libxml_children(ATTRIBUTE_UNUSED PyObject * self, PyObject * args)
{
    PyObject *resultobj, *obj;
    xmlNodePtr cur;
    xmlNodePtr res;

    if (!PyArg_ParseTuple(args, "O:children", &obj))
        return NULL;
    cur = PyxmlNode_Get(obj);

    /* NULL dereference occurs here because cur == NULL */
    switch (cur->type) {
        case XML_ELEMENT_NODE:
        case XML_ENTITY_REF_NODE:
        case XML_ENTITY_NODE:
        case XML_PI_NODE:
        case XML_COMMENT_NODE:
        case XML_DOCUMENT_NODE:
        case XML_HTML_DOCUMENT_NODE:
        case XML_DTD_NODE:
            res = cur->children;
            break;
        case XML_ATTRIBUTE_NODE: {
                xmlAttrPtr attr = (xmlAttrPtr) cur;
                res = attr->children;
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
    /* We don't need real Python objects; our stubs drive the bad path. */
    PyObject *ret;
    PyObject fake_args = { .is_valid = 0 }; /* content ignored by our stub */

    printf("Triggering NULL dereference in libxml_children...\n");
    /* This call will crash in the switch (cur->type) due to cur == NULL */
    ret = libxml_children(NULL, &fake_args);

    /* If the crash didn't happen (unexpected), clean up */
    if (ret) {
        printf("Unexpectedly returned from libxml_children.\n");
        free(ret);
    }
    return 0;
}
