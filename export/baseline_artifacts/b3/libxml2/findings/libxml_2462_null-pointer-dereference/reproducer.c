#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

/* Minimal stubs and type definitions to reproduce the null-pointer dereference
 * in python/libxml.c: libxml_parent().
 *
 * We re-create the minimal environment expected by libxml_parent:
 * - PyObject and PyArg_ParseTuple stub
 * - PyxmlNode_Get stub that returns NULL (simulating an invalid argument)
 * - Minimal xmlNode/xmlAttr types and XML_* constants
 */

#ifndef ATTRIBUTE_UNUSED
#define ATTRIBUTE_UNUSED
#endif

/* Minimal fake PyObject and helpers */
typedef struct {
    void *obj; /* payload for our fake tuple */
} PyObject;

/* Stub: acts like PyArg_ParseTuple(args, "O:parent", &obj)
 * For this reproducer, it just copies args->obj into the provided out pointer. */
int PyArg_ParseTuple(PyObject *args, const char *fmt, void *out) {
    (void)fmt; /* ignore format string */
    if (!args || !out) return 0;
    *(void **)out = args->obj;
    return 1; /* success */
}

/* Minimal libxml node representations */
typedef struct _xmlNode xmlNode;
typedef xmlNode *xmlNodePtr;
struct _xmlNode {
    xmlNodePtr parent;
    xmlNodePtr last;
    int type;
};

typedef struct _xmlAttr {
    /* In real libxml2, xmlAttr is more complex. For our purposes we only need
       parent/last fields consistent with the uses in the vulnerable code. */
    xmlNode node_header; /* not used directly */
    xmlNodePtr parent;
    xmlNodePtr last;
} *xmlAttrPtr;

/* XML node type constants (values don't matter for this crash) */
#define XML_ELEMENT_NODE        1
#define XML_ATTRIBUTE_NODE      2
#define XML_TEXT_NODE           3
#define XML_CDATA_SECTION_NODE  4
#define XML_ENTITY_REF_NODE     5
#define XML_ENTITY_NODE         6
#define XML_PI_NODE             7
#define XML_COMMENT_NODE        8
#define XML_DOCUMENT_NODE       9
#define XML_HTML_DOCUMENT_NODE  13
#define XML_DTD_NODE            14
#define XML_ENTITY_DECL         17
#define XML_NAMESPACE_DECL      18
#define XML_XINCLUDE_START      19
#define XML_XINCLUDE_END        20

/* Stub: In real code this converts a C xmlNodePtr to a Python object. */
PyObject *libxml_xmlNodePtrWrap(xmlNodePtr res) {
    PyObject *o = (PyObject *)malloc(sizeof(PyObject));
    o->obj = (void *)res;
    return o;
}

/* Stub: In real code this extracts an xmlNodePtr from a Python object.
 * To trigger the bug, we return NULL to simulate an invalid argument. */
xmlNodePtr PyxmlNode_Get(PyObject *obj) {
    (void)obj; /* simulate invalid object -> return NULL */
    return NULL;
}

/* Vulnerable function copied/adapted from python/libxml.c */
static PyObject *
libxml_parent(ATTRIBUTE_UNUSED PyObject * self, PyObject * args)
{
    PyObject *resultobj, *obj;
    xmlNodePtr cur;
    xmlNodePtr res;

    if (!PyArg_ParseTuple(args, "O:parent", &obj))
        return NULL;
    cur = PyxmlNode_Get(obj);

    /* BUG: cur may be NULL here; dereferencing cur->type will crash */
    switch (cur->type) {
        case XML_DOCUMENT_NODE:
        case XML_HTML_DOCUMENT_NODE:
            res = NULL;
            break;
        case XML_ATTRIBUTE_NODE: {
                xmlAttrPtr attr = (xmlAttrPtr) cur;
                res = attr->parent;
            }
            break;
        case XML_ENTITY_DECL:
        case XML_NAMESPACE_DECL:
        case XML_XINCLUDE_START:
        case XML_XINCLUDE_END:
            res = NULL;
            break;
        default:
            res = cur->parent;
            break;
    }
    resultobj = libxml_xmlNodePtrWrap(res);
    return resultobj;
}

int main(void) {
    /* Prepare a fake PyObject that acts as the "args" tuple.
     * Our PyArg_ParseTuple stub will extract args->obj into &obj.
     * PyxmlNode_Get will then return NULL regardless, simulating invalid input. */
    PyObject fake_args;
    fake_args.obj = (void *)0xDEADBEEF; /* arbitrary pointer; will be ignored */

    /* This call will dereference a NULL xmlNodePtr inside libxml_parent */
    PyObject *ret = libxml_parent(NULL, &fake_args);

    /* If the program didn't crash (it should), free any allocated object */
    if (ret) free(ret);
    return 0;
}
