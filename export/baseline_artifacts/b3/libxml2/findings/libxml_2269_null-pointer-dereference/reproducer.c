// Standalone C reproducer for NULL pointer dereference in libxml_doc
// Compile with:
//   clang -fsanitize=address -g -O0 reproducer.c -o reproducer \
//          -I/tmp/libxml2_upstream -L/tmp/libxml2_upstream/build/.libs -ltiff -lm

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>

// Minimal stubs and type definitions to mimic the environment expected by python/libxml.c

typedef struct { int dummy; } PyObject;

typedef unsigned char xmlChar;

typedef enum {
    XML_ELEMENT_NODE = 1,
    XML_ATTRIBUTE_NODE = 2,
    XML_DOCUMENT_NODE = 9,
    XML_HTML_DOCUMENT_NODE = 13,
    XML_NAMESPACE_DECL = 18
} xmlElementType;

struct _xmlDoc { int dummy; };
struct _xmlNode {
    xmlElementType type;
    struct _xmlDoc *doc;
    const xmlChar *name;
};
struct _xmlAttr {
    xmlElementType type;
    struct _xmlDoc *doc;
    const xmlChar *name;
};
struct _xmlNs {
    xmlElementType type;
    const xmlChar *prefix;
};

typedef struct _xmlNode* xmlNodePtr;
typedef struct _xmlDoc*  xmlDocPtr;
typedef struct _xmlAttr* xmlAttrPtr;
typedef struct _xmlNs*   xmlNsPtr;

// Stub for PyArg_ParseTuple that simulates extracting a single PyObject (format "O:doc")
// It deliberately sets the extracted object to a non-NULL invalid value to emulate an
// invalid Python object, which will cause PyxmlNode_Get to return NULL.
int PyArg_ParseTuple(PyObject *args, const char *fmt, ...) {
    (void)args; // unused in this stub
    (void)fmt;  // we don't parse the real format string here
    va_list ap;
    va_start(ap, fmt);
    PyObject **objp = va_arg(ap, PyObject **);
    // Provide an invalid but non-NULL object pointer to emulate a bad Python object.
    *objp = (PyObject *)((uintptr_t)0x1);
    va_end(ap);
    return 1; // indicate success so the function continues to the vulnerable path
}

// Stub: converts xmlDocPtr to a PyObject* (never reached if crash occurs first)
PyObject *libxml_xmlDocPtrWrap(xmlDocPtr doc) {
    (void)doc;
    // Return a dummy PyObject to satisfy signature if ever reached
    PyObject *o = (PyObject *)malloc(sizeof(PyObject));
    return o;
}

// Stub: returns NULL to simulate failure to get an xmlNode from an invalid Python object
xmlNodePtr PyxmlNode_Get(PyObject *obj) {
    (void)obj; // In real code, this would check type and possibly extract the node.
    return NULL; // Simulate invalid object => no node
}

// Vulnerable function re-implemented as in python/libxml.c (simplified)
PyObject *libxml_doc(PyObject *self, PyObject *args) {
    (void)self; // ATTRIBUTE_UNUSED in original code
    PyObject *resultobj, *obj;
    xmlNodePtr cur;
    xmlDocPtr res;

    if (!PyArg_ParseTuple(args, "O:doc", &obj))
        return NULL;
    cur = PyxmlNode_Get(obj);

    // Vulnerable NULL dereference: cur is used without a NULL check
    switch (cur->type) {
        case XML_DOCUMENT_NODE:
        case XML_HTML_DOCUMENT_NODE:
            res = NULL;
            break;
        case XML_ATTRIBUTE_NODE: {
            xmlAttrPtr attr = (xmlAttrPtr)cur;
            res = attr->doc;
            break;
        }
        case XML_NAMESPACE_DECL:
            res = NULL;
            break;
        default:
            res = cur->doc;
            break;
    }
    resultobj = libxml_xmlDocPtrWrap(res);
    return resultobj;
}

int main(void) {
    // Create dummy self/args; our PyArg_ParseTuple stub ignores their contents
    PyObject self = {0};
    PyObject args = {0};

    // This call will reach the vulnerable switch with cur == NULL and crash
    PyObject *ret = libxml_doc(&self, &args);
    (void)ret;

    // If the bug does not trigger (it should), print a message
    printf("Unexpectedly returned from libxml_doc\n");
    return 0;
}
