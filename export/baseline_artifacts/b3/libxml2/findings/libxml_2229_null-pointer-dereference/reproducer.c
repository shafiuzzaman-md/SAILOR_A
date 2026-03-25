#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdint.h>

/* Stubs and minimal type definitions to emulate libxml2 + Python C-API pieces */

#ifndef ATTRIBUTE_UNUSED
#define ATTRIBUTE_UNUSED
#endif

typedef unsigned char xmlChar;

enum {
    XML_ELEMENT_NODE = 1,
    XML_ATTRIBUTE_NODE = 2,
    XML_TEXT_NODE = 3,
    XML_CDATA_SECTION_NODE = 4,
    XML_ENTITY_REF_NODE = 5,
    XML_ENTITY_NODE = 6,
    XML_PI_NODE = 7,
    XML_COMMENT_NODE = 8,
    XML_DOCUMENT_NODE = 9,
    XML_DOCUMENT_TYPE_NODE = 10,
    XML_DOCUMENT_FRAG_NODE = 11,
    XML_NOTATION_NODE = 12,
    XML_HTML_DOCUMENT_NODE = 13,
    XML_DTD_NODE = 14,
    XML_ELEMENT_DECL = 15,
    XML_ATTRIBUTE_DECL = 16,
    XML_ENTITY_DECL = 17,
    XML_NAMESPACE_DECL = 18,
    XML_XINCLUDE_START = 19,
    XML_XINCLUDE_END = 20,
    XML_DOCB_DOCUMENT_NODE = 21
};

/* Minimal structs so field access compiles even if not executed */
typedef struct _xmlNode {
    int type;
    const xmlChar *name;
} *xmlNodePtr;

typedef struct _xmlDoc {
    int type;
    const xmlChar *name;
    const xmlChar *URL;
} *xmlDocPtr;

typedef struct _xmlAttr {
    int type;
    const xmlChar *name;
} *xmlAttrPtr;

typedef struct _xmlNs {
    int type;
    const xmlChar *prefix;
} *xmlNsPtr;

/* Super-minimal PyObject emulation */
typedef struct _PyObject {
    int is_tuple;        /* 1 if this is our fake tuple */
    int is_xml_node;     /* 1 if this is an xml node wrapper */
    struct _PyObject *item0; /* first tuple item */
    xmlNodePtr node;     /* wrapped xml node (if any) */
} PyObject;

/* Stubs for functions used by libxml_name */
static int PyArg_ParseTuple(PyObject *args, const char *fmt, ...) {
    /* Emulate only the "O:name" case: extract a single PyObject* */
    va_list ap;
    va_start(ap, fmt);
    PyObject **out_obj = va_arg(ap, PyObject **);
    va_end(ap);

    if (args && args->is_tuple && args->item0) {
        *out_obj = args->item0;
        return 1; /* success */
    }
    *out_obj = NULL;
    return 0; /* fail */
}

static xmlNodePtr PyxmlNode_Get(PyObject *obj) {
    if (obj && obj->is_xml_node && obj->node) {
        return obj->node;
    }
    /* Simulate the problematic behavior: return NULL for invalid object */
    return NULL;
}

static PyObject *libxml_constxmlCharPtrWrap(const xmlChar *res) {
    /* Not reached in this reproducer, but return a dummy object */
    PyObject *o = (PyObject *)calloc(1, sizeof(PyObject));
    return o;
}

/* Vulnerable function reproduced from the snippet */
static PyObject *
libxml_name(ATTRIBUTE_UNUSED PyObject * self, PyObject * args)
{
    PyObject *resultobj, *obj;
    xmlNodePtr cur;
    const xmlChar *res;

    if (!PyArg_ParseTuple(args, "O:name", &obj))
        return NULL;
    cur = PyxmlNode_Get(obj);

    /* NULL dereference here when cur == NULL */
    switch (cur->type) {
        case XML_DOCUMENT_NODE:
        case XML_HTML_DOCUMENT_NODE: {
                xmlDocPtr doc = (xmlDocPtr) cur;
                res = doc->URL;
                break;
            }
        case XML_ATTRIBUTE_NODE: {
                xmlAttrPtr attr = (xmlAttrPtr) cur;
                res = attr->name;
                break;
            }
        case XML_NAMESPACE_DECL: {
                xmlNsPtr ns = (xmlNsPtr) cur;
                res = ns->prefix;
                break;
            }
        default:
            res = cur->name;
            break;
    }
    resultobj = libxml_constxmlCharPtrWrap(res);

    return resultobj;
}

/* Helper constructors for our fake Python objects */
static PyObject *make_non_xml_object(void) {
    PyObject *o = (PyObject *)calloc(1, sizeof(PyObject));
    o->is_tuple = 0;
    o->is_xml_node = 0; /* Not an xml node wrapper */
    o->item0 = NULL;
    o->node = NULL;
    return o;
}

static PyObject *make_tuple_with(PyObject *item0) {
    PyObject *t = (PyObject *)calloc(1, sizeof(PyObject));
    t->is_tuple = 1;
    t->item0 = item0;
    return t;
}

int main(void) {
    /* Create an invalid Python object (not an xml node) */
    PyObject *invalid_obj = make_non_xml_object();

    /* Create args tuple containing the invalid object */
    PyObject *args = make_tuple_with(invalid_obj);

    /* Call the vulnerable function: PyxmlNode_Get returns NULL; cur->type dereferences NULL */
    fprintf(stderr, "About to trigger NULL dereference in libxml_name...\n");
    (void)libxml_name(NULL, args);

    /* If somehow not crashed (shouldn't happen), exit with failure */
    fprintf(stderr, "ERROR: expected crash did not occur.\n");
    return 1;
}
