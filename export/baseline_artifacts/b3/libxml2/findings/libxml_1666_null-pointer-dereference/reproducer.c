#include <stdio.h>
#include <stdlib.h>

// Minimal stand-ins for Python C API and libxml2 types to reproduce the bug

typedef struct _PyObject { int dummy; } PyObject;
typedef long Py_ssize_t;

static PyObject Py_None_obj;
#define Py_None (&Py_None_obj)

#ifndef ATTRIBUTE_UNUSED
#define ATTRIBUTE_UNUSED
#endif

typedef struct _xmlParserCtxt {
    void *_private;
} xmlParserCtxt, *xmlParserCtxtPtr;

typedef struct _xmlParserCtxtPyCtxt {
    PyObject *f;
    PyObject *arg;
} xmlParserCtxtPyCtxt, *xmlParserCtxtPyCtxtPtr;

// Fake "args" container used by our PyArg_ParseTuple stub
typedef struct {
    PyObject *item;
} PyArgs;

// --- Stubs for Python C API ---
int PyArg_ParseTuple(PyObject *args, const char *fmt, PyObject **out) {
    (void)fmt;
    PyArgs *a = (PyArgs*)args;
    *out = a->item;
    return 1; // pretend parse succeeded
}

PyObject *PyTuple_New(Py_ssize_t size) {
    (void)size;
    // Just return a non-NULL pointer
    return (PyObject*)malloc(sizeof(PyObject));
}

int PyTuple_SetItem(PyObject *tuple, Py_ssize_t pos, PyObject *item) {
    (void)tuple; (void)pos; (void)item;
    return 0;
}

void Py_XINCREF(PyObject *o) { (void)o; }
void Py_XDECREF(PyObject *o) { (void)o; }
void Py_INCREF(PyObject *o) { (void)o; }

// --- Stub returning NULL to trigger the vulnerable path ---
void *PyparserCtxt_Get(PyObject *pyobj) {
    (void)pyobj;
    return NULL; // Force ctxt == NULL in the vulnerable function
}

// Vulnerable function (extracted and minimally adapted from python/libxml.c)
static PyObject *
libxml_xmlParserCtxtGetErrorHandler(ATTRIBUTE_UNUSED PyObject *self, PyObject *args) 
{
    PyObject *py_retval;
    xmlParserCtxtPtr ctxt;
    xmlParserCtxtPyCtxtPtr pyCtxt;
    PyObject *pyobj_ctxt;

    if (!PyArg_ParseTuple(args, "O:xmlParserCtxtGetErrorHandler",
                          &pyobj_ctxt))
        return(NULL);
    ctxt = (xmlParserCtxtPtr) PyparserCtxt_Get(pyobj_ctxt);
    py_retval = PyTuple_New(2);
    // BUG: ctxt may be NULL here; dereferencing ctxt->_private causes NPD
    if (ctxt->_private != NULL) {
        pyCtxt = (xmlParserCtxtPyCtxtPtr)ctxt->_private;

        PyTuple_SetItem(py_retval, 0, pyCtxt->f);
        Py_XINCREF(pyCtxt->f);
        PyTuple_SetItem(py_retval, 1, pyCtxt->arg);
        Py_XINCREF(pyCtxt->arg);
    }
    else {
        /* no python error handler registered */
        PyTuple_SetItem(py_retval, 0, Py_None);
        Py_XINCREF(Py_None);
        PyTuple_SetItem(py_retval, 1, Py_None);
        Py_XINCREF(Py_None);
    }
    return(py_retval);
}

int main(void) {
    // Create a dummy Python object that represents a parser context wrapper
    PyObject dummy_ctxt_obj; // Content irrelevant; PyparserCtxt_Get ignores it

    // Package it into our fake args tuple so PyArg_ParseTuple extracts it
    PyArgs fake_args = { .item = &dummy_ctxt_obj };

    // Calling the vulnerable function will attempt to read ctxt->_private
    // with ctxt == NULL, causing a null-pointer dereference.
    PyObject *ret = libxml_xmlParserCtxtGetErrorHandler(NULL, (PyObject*)&fake_args);

    // If it somehow returns (it shouldn't), free the allocated tuple
    free(ret);

    return 0;
}
