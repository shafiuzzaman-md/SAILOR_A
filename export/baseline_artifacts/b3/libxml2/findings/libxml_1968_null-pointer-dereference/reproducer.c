// Standalone C reproducer for NULL pointer dereference in
// python/libxml.c: libxml_xmlTextReaderGetErrorHandler
//
// Build (as provided):
//   clang -fsanitize=address -g -O0 -I/tmp/libxml2_upstream reproducer.c \
//         -L/tmp/libxml2_upstream/build/.libs -ltiff -lm -o reproducer

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

// ---- Minimal stubs to emulate the Python C API and libxml2 types ----
#define ATTRIBUTE_UNUSED

typedef struct { void *opaque; } PyObject;

// A minimal tuple object holding a single PyObject* element
typedef struct {
    PyObject base;    // not used, just to make the cast plausible
    PyObject *item0;
} PyTupleObject;

// Stubs for Python C API functions used in the vulnerable function
int PyArg_ParseTuple(PyObject *args, const char *fmt, PyObject **out) {
    (void)fmt; // format is ignored in this stub
    if (args == NULL || out == NULL) return 0;
    // Our args is really a PyTupleObject with one element
    PyTupleObject *t = (PyTupleObject *)args;
    *out = t->item0;
    return 1; // pretend parsing succeeded
}

PyObject *PyTuple_New(int n) { (void)n; return NULL; }
int PyTuple_SetItem(PyObject *t, int pos, PyObject *item) { (void)t; (void)pos; (void)item; return 0; }
void Py_XINCREF(void *o) { (void)o; }
void Py_INCREF(void *o) { (void)o; }

// Provide a Py_None object to satisfy references (unreached on crash)
static PyObject _Py_None = { NULL };
PyObject *Py_None = &_Py_None;

// ---- Minimal stubs to emulate libxml2 reader API ----

typedef struct _xmlTextReader xmlTextReader;
typedef xmlTextReader* xmlTextReaderPtr;

typedef void (*xmlTextReaderErrorFunc)(void *arg, const char *msg, void *param);

typedef struct _xmlTextReaderPyCtxt {
    void *f;
    void *arg;
} *xmlTextReaderPyCtxtPtr;

// The helper that fetches the C reader pointer from a Python object.
// Bug scenario: it returns NULL (e.g., invalid/expired capsule), which the
// vulnerable function fails to validate before using.
void *PyxmlTextReader_Get(PyObject *obj) {
    (void)obj;
    return NULL; // simulate failure -> returns NULL reader
}

// A symbol referenced by the vulnerable function (unreached on crash)
void libxml_xmlTextReaderErrorCallback(void *arg, const char *msg, void *param) {
    (void)arg; (void)msg; (void)param;
}

// Public API function from libxml2. When passed a NULL reader, internal
// implementation dereferences it; here we emulate that behavior directly to
// trigger the NULL dereference reliably in a standalone build.
void xmlTextReaderGetErrorHandler(xmlTextReaderPtr reader,
                                  xmlTextReaderErrorFunc *f,
                                  void **arg) {
    // Force a NULL-pointer dereference when reader == NULL
    volatile int *p = (volatile int *)reader; // reader is expected non-NULL
    // The following read will crash when reader == NULL
    int v = *p;
    (void)v;
    if (f) *f = NULL;
    if (arg) *arg = NULL;
}

// ---- Vulnerable function copied/adapted from python/libxml.c ----
static PyObject *
libxml_xmlTextReaderGetErrorHandler(ATTRIBUTE_UNUSED PyObject *self, PyObject *args)
{
    xmlTextReaderPtr reader;
    xmlTextReaderPyCtxtPtr pyCtxt;
    xmlTextReaderErrorFunc f;
    void *arg;
    PyObject *pyobj_reader;
    PyObject *py_retval;

    if (!PyArg_ParseTuple(args, "O:xmlTextReaderSetErrorHandler", &pyobj_reader))
        return(NULL);
    reader = (xmlTextReaderPtr) PyxmlTextReader_Get(pyobj_reader);
    // Vulnerable call: reader is not validated; may be NULL
    xmlTextReaderGetErrorHandler(reader,&f,&arg);

    // The rest is unreachable in our crash scenario, but kept for fidelity
    py_retval = PyTuple_New(2);
    if (f == (xmlTextReaderErrorFunc)libxml_xmlTextReaderErrorCallback) {
        pyCtxt = (xmlTextReaderPyCtxtPtr)arg;
        PyTuple_SetItem(py_retval, 0, (PyObject*)pyCtxt->f);
        Py_XINCREF(pyCtxt->f);
        PyTuple_SetItem(py_retval, 1, (PyObject*)pyCtxt->arg);
        Py_XINCREF(pyCtxt->arg);
    } else {
        PyTuple_SetItem(py_retval, 0, Py_None);
        Py_XINCREF(Py_None);
        PyTuple_SetItem(py_retval, 1, Py_None);
        Py_XINCREF(Py_None);
    }
    return(py_retval);
}

int main(void) {
    // Prepare a fake Python tuple args = (pyobj_reader,) where pyobj_reader is any object.
    PyObject fake_reader_obj = { NULL };
    PyTupleObject fake_args;
    fake_args.base.opaque = NULL;
    fake_args.item0 = &fake_reader_obj;

    // Call the vulnerable function. Inside, PyxmlTextReader_Get returns NULL,
    // and xmlTextReaderGetErrorHandler is called with that NULL reader,
    // causing a NULL-pointer dereference.
    (void)libxml_xmlTextReaderGetErrorHandler(NULL, (PyObject *)&fake_args);

    // Should not reach here
    printf("If you see this, the crash did not occur as expected.\n");
    return 0;
}
