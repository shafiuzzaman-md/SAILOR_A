// Standalone C reproducer for null-pointer-dereference in
// libxml_xmlTextReaderSetErrorHandler due to missing NULL check on reader
// before calling xmlTextReaderGetErrorHandler(reader,...)

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdint.h>

// ------- Minimal Python C-API stubs -------

typedef struct PyObject PyObject;

enum { PY_TUPLE = 1, PY_INT, PY_STR, PY_NONE, PY_FUNC, PY_READER };

struct PyTuple {
    int size;
    PyObject **items;
};

struct PyObject {
    int type;
    int refcnt;
    void *ptr; // points to type-specific payload (e.g., struct PyTuple*)
};

static PyObject Py_None_Singleton = { PY_NONE, 1, NULL };
#define Py_None (&Py_None_Singleton)

static PyObject *new_pyobject(int type, void *payload) {
    PyObject *o = (PyObject *)calloc(1, sizeof(PyObject));
    if (!o) { perror("calloc"); exit(1); }
    o->type = type;
    o->refcnt = 1;
    o->ptr = payload;
    return o;
}

static PyObject *PyTuple_New(int n) {
    struct PyTuple *t = (struct PyTuple *)calloc(1, sizeof(struct PyTuple));
    if (!t) { perror("calloc"); exit(1); }
    t->size = n;
    t->items = (PyObject **)calloc((size_t)n, sizeof(PyObject *));
    if (!t->items) { perror("calloc"); exit(1); }
    return new_pyobject(PY_TUPLE, t);
}

static int PyTuple_SetItem(PyObject *tuple, int index, PyObject *item) {
    if (!tuple || tuple->type != PY_TUPLE) return -1;
    struct PyTuple *t = (struct PyTuple *)tuple->ptr;
    if (index < 0 || index >= t->size) return -1;
    t->items[index] = item; // Ignore refcount semantics for simplicity
    return 0;
}

static int PyArg_ParseTuple(PyObject *args, const char *format, ...) {
    if (!args || args->type != PY_TUPLE) return 0;
    // This stub only supports the format used by the vulnerable function
    // "OOO:xmlTextReaderSetErrorHandler"
    (void)format; // accept any string to be forgiving
    struct PyTuple *t = (struct PyTuple *)args->ptr;
    if (!t || t->size < 3) return 0;
    va_list ap;
    va_start(ap, format);
    PyObject **out1 = va_arg(ap, PyObject **);
    PyObject **out2 = va_arg(ap, PyObject **);
    PyObject **out3 = va_arg(ap, PyObject **);
    if (out1) *out1 = t->items[0];
    if (out2) *out2 = t->items[1];
    if (out3) *out3 = t->items[2];
    va_end(ap);
    return 1;
}

static PyObject *PyObject_CallObject(PyObject *callable, PyObject *args) {
    (void)callable; (void)args;
    return NULL; // Not used in this reproducer path
}

static void PyErr_Print(void) { fprintf(stderr, "PyErr_Print called (stub)\n"); }
static void Py_XINCREF(PyObject *o) { if (o) o->refcnt++; }
static void Py_XDECREF(PyObject *o) { if (!o) return; if (--o->refcnt <= 0) { free(o->ptr); free(o); } }

// Minimal wrappers used by the original code
static PyObject *libxml_charPtrConstWrap(const char *s) {
    // store a strdup so freeing works if decref'd
    char *dup = s ? strdup(s) : NULL;
    return new_pyobject(PY_STR, dup);
}
static PyObject *libxml_intWrap(int v) {
    int *pv = (int *)malloc(sizeof(int));
    if (!pv) { perror("malloc"); exit(1); }
    *pv = v;
    return new_pyobject(PY_INT, pv);
}
static PyObject *libxml_xmlTextReaderLocatorPtrWrap(void *loc) {
    return new_pyobject(PY_INT, loc); // opaque
}

// ------- Minimal libxml2 reader-related stubs -------

typedef struct _xmlTextReader {
    int dummy;
} xmlTextReader;

typedef xmlTextReader *xmlTextReaderPtr;

typedef void *xmlTextReaderLocatorPtr;

typedef enum { XML_PARSER_SEVERITY_NONE = 0 } xmlParserSeverities;

typedef void (*xmlTextReaderErrorFunc)(void *arg, const char *msg, xmlParserSeverities severity, xmlTextReaderLocatorPtr locator);

// Context type used by the binding
typedef struct _xmlTextReaderPyCtxt {
    PyObject *f;
    PyObject *arg;
} xmlTextReaderPyCtxt, *xmlTextReaderPyCtxtPtr;

// Allocation wrappers
static void *xmlMalloc(size_t s) { return malloc(s); }
static void xmlFree(void *p) { free(p); }

// This function will dereference the reader pointer, thus crashing on NULL
static int xmlTextReaderGetErrorHandler(xmlTextReaderPtr reader, xmlTextReaderErrorFunc *f, void **arg) {
    // Intentional dereference to emulate real libxml2 behavior (reader must be non-NULL)
    volatile int x = ((xmlTextReader *)reader)->dummy; // NULL deref here when reader == NULL
    (void)x;
    if (f) *f = NULL;
    if (arg) *arg = NULL;
    return 0;
}

static int xmlTextReaderSetErrorHandler(xmlTextReaderPtr reader, xmlTextReaderErrorFunc func, void *arg) {
    (void)reader; (void)func; (void)arg;
    return 0;
}

// Stub for invalid reader extraction: returns NULL to simulate bad Python object
static void *PyxmlTextReader_Get(PyObject *pyobj_reader) {
    // Only objects with type PY_READER are considered valid; all others -> NULL
    if (!pyobj_reader || pyobj_reader->type != PY_READER) return NULL;
    return pyobj_reader->ptr; // not used in this reproducer
}

// Error callback (not invoked in this reproducer but referenced by the code)
static void libxml_xmlTextReaderErrorCallback(void *arg, const char *msg, xmlParserSeverities severity, xmlTextReaderLocatorPtr locator) {
    (void)arg; (void)msg; (void)severity; (void)locator;
}

#ifndef ATTRIBUTE_UNUSED
#define ATTRIBUTE_UNUSED __attribute__((unused))
#endif

// ------- Vulnerable function copied/adapted from the binding -------
static PyObject *
libxml_xmlTextReaderSetErrorHandler(ATTRIBUTE_UNUSED PyObject *self, PyObject *args)
{
    xmlTextReaderPtr reader;
    xmlTextReaderPyCtxtPtr pyCtxt;
    xmlTextReaderErrorFunc f;
    void *arg;
    PyObject *pyobj_reader;
    PyObject *pyobj_f;
    PyObject *pyobj_arg;
    PyObject *py_retval;

    if (!PyArg_ParseTuple(args, "OOO:xmlTextReaderSetErrorHandler", &pyobj_reader, &pyobj_f, &pyobj_arg))
        return(NULL);
    reader = (xmlTextReaderPtr) PyxmlTextReader_Get(pyobj_reader);
    // clear previous error handler
    // BUG: reader is not checked for NULL before this call
    xmlTextReaderGetErrorHandler(reader,&f,&arg); // NULL deref when reader == NULL
    if (arg != NULL) {
        if (f == (xmlTextReaderErrorFunc) libxml_xmlTextReaderErrorCallback) {
            // ok, it's our error handler!
            pyCtxt = (xmlTextReaderPyCtxtPtr)arg;
            Py_XDECREF(pyCtxt->f);
            Py_XDECREF(pyCtxt->arg);
            xmlFree(pyCtxt);
        }
        else {
            // existing arg that's not ours -> bail out
            py_retval = libxml_intWrap(-1);
            return(py_retval);
        }
    }
    xmlTextReaderSetErrorHandler(reader,NULL,NULL);
    // set new error handler
    if (pyobj_f != Py_None)
    {
        pyCtxt = (xmlTextReaderPyCtxtPtr)xmlMalloc(sizeof(xmlTextReaderPyCtxt));
        if (pyCtxt == NULL) {
            py_retval = libxml_intWrap(-1);
            return(py_retval);
        }
        Py_XINCREF(pyobj_f);
        pyCtxt->f = pyobj_f;
        Py_XINCREF(pyobj_arg);
        pyCtxt->arg = pyobj_arg;
        xmlTextReaderSetErrorHandler(reader,
                                     libxml_xmlTextReaderErrorCallback,
                                     pyCtxt);
    }

    py_retval = libxml_intWrap(1);
    return(py_retval);
}

int main(void) {
    // Create a tuple of 3 Python objects: reader, f, arg
    // We intentionally pass an INVALID reader object to force PyxmlTextReader_Get to return NULL.
    PyObject *args = PyTuple_New(3);

    // Invalid reader (type not PY_READER) -> PyxmlTextReader_Get returns NULL
    PyObject *invalid_reader = new_pyobject(PY_INT, NULL);
    // Handler function (could be None; not reached before the crash)
    PyObject *handler = Py_None;
    // User arg object
    PyObject *user_arg = new_pyobject(PY_STR, strdup("ctx"));

    PyTuple_SetItem(args, 0, invalid_reader);
    PyTuple_SetItem(args, 1, handler);
    PyTuple_SetItem(args, 2, user_arg);

    // Call the vulnerable function; it will attempt to get the error handler
    // using a NULL reader and crash inside xmlTextReaderGetErrorHandler.
    libxml_xmlTextReaderSetErrorHandler(NULL, args);

    // Should not reach here
    printf("Unexpectedly returned from vulnerable call\n");
    return 0;
}
