#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

/* Self-contained stubs and minimal type definitions to reproduce the bug */

/* ATTRIBUTE_UNUSED as in the original code */
#ifndef ATTRIBUTE_UNUSED
#define ATTRIBUTE_UNUSED
#endif

/* Minimal Python object representation */
typedef struct _PyObject {
    int dummy;
} PyObject;

/* Provide a Py_None like object */
static PyObject _Py_NoneStruct;
static PyObject *Py_None = &_Py_NoneStruct;

/* Minimal xmlParserCtxt and related types */
typedef struct _xmlParserCtxt {
    void *_private; /* this is dereferenced without checking ctxt for NULL */
} xmlParserCtxt, *xmlParserCtxtPtr;

/* The per-context Python callback holder used by the original code */
typedef struct _xmlParserCtxtPyCtxt {
    PyObject *f;
    PyObject *arg;
} xmlParserCtxtPyCtxt, *xmlParserCtxtPyCtxtPtr;

/* Stubs for functions/macros used in the vulnerable function */
static int PyArg_ParseTuple(PyObject *args, const char *format, ...) {
    /* Ignore args and format; just fill out three PyObject* outputs */
    (void)args;
    (void)format;
    va_list ap;
    va_start(ap, format);
    PyObject **p1 = va_arg(ap, PyObject **);
    PyObject **p2 = va_arg(ap, PyObject **);
    PyObject **p3 = va_arg(ap, PyObject **);
    va_end(ap);
    static PyObject dummy_ctxt, dummy_f, dummy_arg;
    if (p1) *p1 = &dummy_ctxt;
    if (p2) *p2 = &dummy_f;
    if (p3) *p3 = &dummy_arg;
    return 1; /* success */
}

/* This function is the root cause: it can return NULL for invalid context objects */
static void *PyparserCtxt_Get(PyObject *pyobj_ctxt) {
    (void)pyobj_ctxt;
    /* Simulate an invalid/non-context Python object by returning NULL */
    return NULL;
}

static void *xmlMalloc(size_t size) { return malloc(size); }

static void Py_XDECREF(PyObject *o) { (void)o; }
static void Py_XINCREF(PyObject *o) { (void)o; }
static void Py_INCREF(PyObject *o) { (void)o; }
static void PyErr_Print(void) { fprintf(stderr, "PyErr_Print() called\n"); }

static PyObject *libxml_intWrap(int v) {
    /* Return a unique address per call isn't necessary; use a static */
    static PyObject obj;
    (void)v;
    return &obj;
}

/* Error handler type and stub used below (won't be reached in this PoC) */
typedef void (*xmlErrorHandler)(void *ctxt, void *userData, void *error);
static void libxml_xmlParserCtxtErrorHandler(void *ctxt, void *userData, void *error) {
    (void)ctxt; (void)userData; (void)error;
}
static void xmlCtxtSetErrorHandler(xmlParserCtxtPtr ctxt, xmlErrorHandler handler, void *data) {
    (void)ctxt; (void)handler; (void)data;
}

/* Vulnerable function, reproduced from the source context */
static PyObject *
libxml_xmlParserCtxtSetErrorHandler(ATTRIBUTE_UNUSED PyObject *self, PyObject *args)
{
    PyObject *py_retval;
    xmlParserCtxtPtr ctxt;
    xmlParserCtxtPyCtxtPtr pyCtxt;
    PyObject *pyobj_ctxt;
    PyObject *pyobj_f;
    PyObject *pyobj_arg;

    if (!PyArg_ParseTuple(args, "OOO:xmlParserCtxtSetErrorHandler",
                          &pyobj_ctxt, &pyobj_f, &pyobj_arg))
        return NULL;
    ctxt = (xmlParserCtxtPtr) PyparserCtxt_Get(pyobj_ctxt);
    /* BUG: ctxt may be NULL here. The next line dereferences ctxt->_private. */
    if (ctxt->_private == NULL) {
        pyCtxt = (xmlParserCtxtPyCtxtPtr) xmlMalloc(sizeof(xmlParserCtxtPyCtxt));
        if (pyCtxt == NULL) {
            py_retval = libxml_intWrap(-1);
            return py_retval;
        }
        memset(pyCtxt, 0, sizeof(xmlParserCtxtPyCtxt));
        ctxt->_private = pyCtxt;
    }
    else {
        pyCtxt = (xmlParserCtxtPyCtxtPtr)ctxt->_private;
    }
    /* The rest is not reached in this reproducer */
    Py_XDECREF(pyCtxt->f);
    Py_XINCREF(pyobj_f);
    pyCtxt->f = pyobj_f;
    Py_XDECREF(pyCtxt->arg);
    Py_XINCREF(pyobj_arg);
    pyCtxt->arg = pyobj_arg;

    if (pyobj_f != Py_None) {
        xmlCtxtSetErrorHandler(ctxt, libxml_xmlParserCtxtErrorHandler, ctxt);
    }
    else {
        xmlCtxtSetErrorHandler(ctxt, NULL, NULL);
    }

    py_retval = libxml_intWrap(1);
    return py_retval;
}

int main(void) {
    /* Passing NULL as args is fine because our PyArg_ParseTuple stub ignores it
       and sets up the outputs. PyparserCtxt_Get will return NULL, triggering
       the NULL dereference on ctxt->_private. */
    libxml_xmlParserCtxtSetErrorHandler(NULL, NULL);
    /* If the program hasn't crashed yet (it should), exit nonzero. */
    return 0;
}
