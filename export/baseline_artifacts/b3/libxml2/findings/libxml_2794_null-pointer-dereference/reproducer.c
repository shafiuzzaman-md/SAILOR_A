#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

/* Minimal type re-declarations to mimic libxml2/python glue */
typedef unsigned char xmlChar;

typedef struct _xmlDoc xmlDoc;
typedef struct _xmlNode xmlNode;

typedef xmlDoc* xmlDocPtr;
typedef xmlNode* xmlNodePtr;

typedef struct _xmlOutputBuffer xmlOutputBuffer;
typedef xmlOutputBuffer* xmlOutputBufferPtr;

typedef void* xmlCharEncodingHandlerPtr;

struct _xmlDoc {
    int type; /* only field we need */
};

struct _xmlNode {
    void* _private;
    int type;      /* node type */
    xmlDocPtr doc; /* owning document, can be NULL */
};

struct _xmlOutputBuffer { int dummy; };

/* Node type constants (values chosen to be distinct) */
#define XML_ELEMENT_NODE          1
#define XML_DOCUMENT_NODE         9
#define XML_HTML_DOCUMENT_NODE   13

/* Python C-API stubs */
typedef struct { int dummy; } PyObject;

/* Structure used to pass fake Python tuple args into PyArg_ParseTuple */
typedef struct {
    PyObject* pyobj_node;
    PyObject* py_file;
    const char* encoding;
    int format;
} SaveArgs;

/* Stub: parse our fake args and populate out-parameters */
int PyArg_ParseTuple(PyObject* args, const char* fmt, ...) {
    (void)fmt; /* ignore the format string */
    SaveArgs* a = (SaveArgs*)args;
    va_list ap;
    va_start(ap, fmt);
    PyObject** out_node = va_arg(ap, PyObject**);
    PyObject** out_file = va_arg(ap, PyObject**);
    const char** out_enc = va_arg(ap, const char**);
    int* out_fmt = va_arg(ap, int*);
    va_end(ap);
    if (!a || !out_node || !out_file || !out_enc || !out_fmt)
        return 0;
    *out_node = a->pyobj_node;
    *out_file = a->py_file;
    *out_enc = a->encoding;
    *out_fmt = a->format;
    return 1;
}

/* Stub: convert long to PyObject* (we just return a heap number) */
PyObject* PyLong_FromLong(long v) {
    long* p = (long*)malloc(sizeof(long));
    if (p) *p = v;
    return (PyObject*)p;
}

/* Stub: get a FILE* from a PyObject* (we pass FILE* directly) */
void* PyFile_Get(PyObject* py_file) {
    return (void*)py_file;
}

/* Stub: release file (no-op) */
void PyFile_Release(void* f) {
    (void)f;
}

/* Stub: unwrap Python xml node object to xmlNodePtr */
xmlNodePtr PyxmlNode_Get(PyObject* obj) {
    return (xmlNodePtr)obj;
}

/* libxml2 I/O and encoding stubs (unreached before the crash) */
xmlCharEncodingHandlerPtr xmlFindCharEncodingHandler(const char* enc) {
    (void)enc; return (xmlCharEncodingHandlerPtr)0x1; /* non-NULL */
}

xmlOutputBufferPtr xmlOutputBufferCreateFile(void* file, xmlCharEncodingHandlerPtr handler) {
    (void)file; (void)handler; return (xmlOutputBufferPtr)malloc(sizeof(xmlOutputBuffer));
}

int xmlSaveFormatFileTo(xmlOutputBufferPtr buf, xmlDocPtr doc, const char* encoding, int format) {
    (void)buf; (void)doc; (void)encoding; (void)format; return 0;
}

void htmlDocContentDumpFormatOutput(xmlOutputBufferPtr buf, xmlDocPtr doc, const char* encoding, int format) {
    (void)buf; (void)doc; (void)encoding; (void)format;
}

int xmlOutputBufferClose(xmlOutputBufferPtr buf) {
    if (buf) free(buf);
    return 0;
}

void htmlNodeDumpFormatOutput(xmlOutputBufferPtr buf, xmlDocPtr doc, xmlNodePtr node, const char* encoding, int format) {
    (void)buf; (void)doc; (void)node; (void)encoding; (void)format;
}

void xmlNodeDumpOutput(xmlOutputBufferPtr buf, xmlDocPtr doc, xmlNodePtr node, int level, int format, const char* encoding) {
    (void)buf; (void)doc; (void)node; (void)level; (void)format; (void)encoding;
}

/* Vulnerable function stubbed from python/libxml.c */
PyObject* libxml_saveNodeTo(PyObject* self, PyObject* args) {
    (void)self;
    PyObject* pyobj_node;
    PyObject* py_file;
    void* output;
    xmlNodePtr node;
    xmlDocPtr doc;
    const char* encoding;
    int format;
    int len;
    xmlOutputBufferPtr buf;
    xmlCharEncodingHandlerPtr handler = NULL;

    if (!PyArg_ParseTuple(args, "OOzi:serializeNode", &pyobj_node, &py_file, &encoding, &format))
        return NULL;
    node = (xmlNodePtr)PyxmlNode_Get(pyobj_node);
    if (node == NULL) {
        return PyLong_FromLong((long)-1);
    }
    output = PyFile_Get(py_file);
    if (output == NULL) {
        return PyLong_FromLong((long)-1);
    }

    if (node->type == XML_DOCUMENT_NODE) {
        doc = (xmlDocPtr)node;
    } else if (node->type == XML_HTML_DOCUMENT_NODE) {
        doc = (xmlDocPtr)node;
    } else {
        doc = node->doc; /* doc can be NULL for unattached nodes */
    }

    /* The first HTML block is behind a feature macro in the real source.
       We omit it here to mirror a typical build where the next check is reached. */

    if (encoding != NULL) {
        handler = xmlFindCharEncodingHandler(encoding);
        if (handler == NULL) {
            PyFile_Release(output);
            return PyLong_FromLong((long)-1);
        }
    }

    /* Unconditional dereference of doc->type without NULL check -> crash */
    if (doc->type == XML_HTML_DOCUMENT_NODE) {
        if (handler == NULL)
            handler = xmlFindCharEncodingHandler("HTML");
        if (handler == NULL)
            handler = xmlFindCharEncodingHandler("ascii");
    }

    /* Rest of function (unreached in this reproducer) */
    buf = xmlOutputBufferCreateFile(output, handler);
    if (node->type == XML_DOCUMENT_NODE) {
        len = xmlSaveFormatFileTo(buf, doc, encoding, format);
    } else {
        xmlNodeDumpOutput(buf, doc, node, 0, format, encoding);
        len = xmlOutputBufferClose(buf);
    }
    PyFile_Release(output);
    return PyLong_FromLong((long)len);
}

/* Minimal creator that returns a non-document node with node->doc == NULL */
PyObject* libxml_xmlNewNode(PyObject* self, PyObject* args) {
    (void)self; (void)args;
    xmlNodePtr node = (xmlNodePtr)malloc(sizeof(*node));
    if (!node) return NULL;
    node->type = XML_ELEMENT_NODE; /* non-document node */
    node->doc = NULL;              /* unattached -> triggers bug */
    node->_private = NULL;
    return (PyObject*)node;        /* directly return as PyObject stub */
}

int main(void) {
    /* Step 1: Create a new node not attached to any document (doc == NULL) */
    PyObject* new_node = libxml_xmlNewNode(NULL, NULL);

    /* Step 2: Arrange fake Python args to pass into libxml_saveNodeTo */
    SaveArgs call_args;
    call_args.pyobj_node = new_node;        /* will be unwrapped to xmlNodePtr */
    call_args.py_file = (PyObject*)stdout;  /* any non-NULL file handle */
    call_args.encoding = NULL;              /* keep encoding NULL to skip handler lookup */
    call_args.format = 1;                   /* pretty print flag */

    /* Step 3: Call the vulnerable function; this will dereference doc->type where doc == NULL */
    (void)libxml_saveNodeTo(NULL, (PyObject*)&call_args);

    /* Should not reach here; if it does, exit non-zero to indicate unexpected behavior */
    fprintf(stderr, "Unexpectedly survived null dereference.\n");
    return 1;
}
