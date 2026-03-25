#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdint.h>

/* Self-contained stubs for a minimal subset of Python C-API and libxml2 glue */

#define ATTRIBUTE_UNUSED

/* Minimal PyObject layout */
typedef struct {
    size_t ob_refcnt;
    void *ob_type;
} PyObject;

#define PyObject_HEAD size_t ob_refcnt; void *ob_type

/* Forward-declare libxml-ish pointer types */
typedef void* xmlOutputBufferPtr;
typedef void* xmlDocPtr;

/* The extension's wrapper type for output buffers */
typedef struct {
    PyObject_HEAD;
    xmlOutputBufferPtr obj;
} PyoutputBuffer_Object;

/* Fake Python types to identify objects */
typedef struct { const char *name; } FakeType;
static FakeType Type_OutputBuffer = { "OutputBuffer" };
static FakeType Type_Other        = { "Other" };

/* A fake tuple object used to pass arguments to PyArg_ParseTuple */
typedef struct {
    PyObject base;     /* so we can pass it as a PyObject* */
    PyObject *a;       /* pyobj_buf */
    PyObject *b;       /* pyobj_cur */
    char *s;           /* encoding */
} FakeTuple;

/* Variadic stub matching the signature used by the vulnerable function */
static int PyArg_ParseTuple(PyObject *args, const char *fmt, ...) {
    (void)fmt; /* ignore format in this stub */
    FakeTuple *t = (FakeTuple *)args;
    va_list ap;
    va_start(ap, fmt);
    PyObject **out1 = va_arg(ap, PyObject **); /* pyobj_buf */
    PyObject **out2 = va_arg(ap, PyObject **); /* pyobj_cur */
    char **out3     = va_arg(ap, char **);     /* encoding */
    va_end(ap);
    if (!t || !out1 || !out2 || !out3) return 0;
    *out1 = t->a;
    *out2 = t->b;
    *out3 = t->s;
    return 1;
}

/* Stubs for helper accessors used by the binding */
static void *PyoutputBuffer_Get(PyObject *obj) {
    if (!obj) return NULL;
    if (obj->ob_type == &Type_OutputBuffer) {
        return (void *)((PyoutputBuffer_Object *)obj)->obj;
    }
    /* If not the expected type, real code returns NULL */
    return NULL;
}

static void *PyxmlNode_Get(PyObject *obj) {
    /* Return some non-NULL dummy document pointer */
    (void)obj;
    return (void *)0x1;
}

/* Stub for xmlSaveFileTo: just succeed without touching buf */
static int xmlSaveFileTo(xmlOutputBufferPtr buf, xmlDocPtr cur, const char *encoding) {
    (void)buf; (void)cur; (void)encoding;
    return 0; /* pretend success */
}

/* Stub for libxml_intWrap: return a dummy PyObject */
static PyObject *libxml_intWrap(int v) {
    PyObject *o = (PyObject *)malloc(sizeof(PyObject));
    if (!o) exit(1);
    o->ob_refcnt = (size_t)v;
    o->ob_type = &Type_Other;
    return o;
}

/* ---------------- Vulnerable function copied/adapted from python/libxml.c ---------------- */
static PyObject *
libxml_xmlSaveFileTo(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *py_retval;
    int c_retval;
    xmlOutputBufferPtr buf;
    PyObject *pyobj_buf;
    xmlDocPtr cur;
    PyObject *pyobj_cur;
    char *encoding;

    if (!PyArg_ParseTuple(args, "OOz:xmlSaveFileTo", &pyobj_buf, &pyobj_cur, &encoding))
        return NULL;
    buf = (xmlOutputBufferPtr) PyoutputBuffer_Get(pyobj_buf);
    cur = (xmlDocPtr) PyxmlNode_Get(pyobj_cur);

    c_retval = xmlSaveFileTo(buf, cur, encoding);
    /* xmlSaveTo() freed the memory pointed to by buf, so record that in the
     * Python object. The bug: unconditionally cast and write into pyobj_buf. */
    ((PyoutputBuffer_Object *)(pyobj_buf))->obj = NULL; /* OOB write if pyobj_buf is not big enough */
    py_retval = libxml_intWrap((int) c_retval);
    return py_retval;
}
/* ---------------------------------------------------------------------------------------- */

int main(void) {
    /* Allocate a tiny PyObject (no room for the 'obj' field of PyoutputBuffer_Object) */
    PyObject *non_output_buffer_obj = (PyObject *)malloc(sizeof(PyObject));
    if (!non_output_buffer_obj) return 1;
    non_output_buffer_obj->ob_refcnt = 1;
    non_output_buffer_obj->ob_type = &Type_Other; /* Not an output buffer type */

    /* Dummy current document object (unused by our stubs) */
    PyObject *doc_obj = (PyObject *)malloc(sizeof(PyObject));
    if (!doc_obj) return 1;
    doc_obj->ob_refcnt = 1;
    doc_obj->ob_type = &Type_Other;

    /* Prepare fake tuple args: (pyobj_buf, pyobj_cur, encoding) */
    FakeTuple tuple;
    memset(&tuple, 0, sizeof(tuple));
    tuple.base.ob_refcnt = 1;
    tuple.base.ob_type = &Type_Other; /* any type */
    tuple.a = non_output_buffer_obj;  /* pyobj_buf: NOT an output buffer */
    tuple.b = doc_obj;                /* pyobj_cur */
    tuple.s = NULL;                   /* encoding */

    /* Call the vulnerable function. During the assignment to obj, it will write
       past the end of non_output_buffer_obj, triggering ASan OOB-write. */
    PyObject *ret = libxml_xmlSaveFileTo(NULL, (PyObject *)&tuple);

    /* Prevent unused warning and keep objects alive a bit. */
    if (ret) {
        /* Clean up to avoid leaks if we reached here without ASan aborting */
        free(ret);
    }
    free(doc_obj);
    free(non_output_buffer_obj);

    return 0;
}
