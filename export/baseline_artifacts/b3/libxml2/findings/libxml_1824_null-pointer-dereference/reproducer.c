#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

/* Minimal stubs to mimic the Python and libxml2 pieces used by the vulnerable code */

#ifndef ATTRIBUTE_UNUSED
# if defined(__GNUC__)
#  define ATTRIBUTE_UNUSED __attribute__((unused))
# else
#  define ATTRIBUTE_UNUSED
# endif
#endif

/* --- Fake Python C-API --- */
typedef struct _PyObject {
    int tag;
} PyObject;

static PyObject _Py_NoneStruct = {0};
#define Py_None (&_Py_NoneStruct)

#define Py_XDECREF(op) do { (void)(op); } while(0)
#define Py_XINCREF(op) do { (void)(op); } while(0)
#define Py_INCREF(op)  do { (void)(op); } while(0)

/* Tuple-like object to carry parameters into PyArg_ParseTuple stub */
typedef struct {
    PyObject base;
    PyObject *items[4];
    int count;
} PyTupleArgs;

static int PyArg_ParseTuple(PyObject *args, const char *fmt ATTRIBUTE_UNUSED,
                            PyObject **o1, PyObject **o2, PyObject **o3, PyObject **o4)
{
    PyTupleArgs *t = (PyTupleArgs *)args;
    if (!t || t->count < 3)
        return 0;
    if (o1) *o1 = t->items[0];
    if (o2) *o2 = t->items[1];
    if (o3) *o3 = t->items[2];
    if (o4) *o4 = (t->count >= 4) ? t->items[3] : NULL;
    return 1;
}

static PyObject *libxml_intWrap(int v)
{
    PyObject *o = (PyObject *)malloc(sizeof(PyObject));
    if (o) o->tag = v;
    return o;
}

/* --- Minimal libxml2 validation context and helpers --- */
typedef struct _xmlValidCtxt {
    void (*error)(void *ctx, const char *msg, ...);
    void (*warning)(void *ctx, const char *msg, ...);
    void *userData;
} xmlValidCtxt, *xmlValidCtxtPtr;

typedef struct _xmlValidCtxtPyCtxt {
    PyObject *error;
    PyObject *warn;
    PyObject *arg;
} xmlValidCtxtPyCtxt, *xmlValidCtxtPyCtxtPtr;

static void *xmlMalloc(size_t sz) { return malloc(sz); }
static void xmlFree(void *p) { free(p); }
static void xmlFreeValidCtxt(xmlValidCtxtPtr p) { free(p); }

static void libxml_xmlValidCtxtErrorFuncHandler(void *ctx ATTRIBUTE_UNUSED, const char *msg ATTRIBUTE_UNUSED, ...) {}
static void libxml_xmlValidCtxtWarningFuncHandler(void *ctx ATTRIBUTE_UNUSED, const char *msg ATTRIBUTE_UNUSED, ...) {}

/* This is the critical helper: returning NULL simulates an invalid validator context
   coming from Python, which the vulnerable code fails to check before dereferencing. */
static xmlValidCtxtPtr PyValidCtxt_Get(PyObject *obj ATTRIBUTE_UNUSED)
{
    /* Always return NULL to trigger the null-pointer dereference in the vulnerable code */
    return NULL;
}

/* ---------------- Vulnerable function (mirrors the relevant parts) ---------------- */
static PyObject *
libxml_xmlSetValidErrors(ATTRIBUTE_UNUSED PyObject * self, PyObject * args)
{
    PyObject *py_retval;
    PyObject *pyobj_error;
    PyObject *pyobj_warn;
    PyObject *pyobj_ctx;
    PyObject *pyobj_arg = Py_None;
    xmlValidCtxtPtr ctxt;
    xmlValidCtxtPyCtxtPtr pyCtxt;

    if (!PyArg_ParseTuple(args, "OOO|O:xmlSetValidErrors", &pyobj_ctx, &pyobj_error, &pyobj_warn, &pyobj_arg))
        return (NULL);

    /* BUG: ctxt may be NULL (invalid validator context from Python) */
    ctxt = PyValidCtxt_Get(pyobj_ctx);

    pyCtxt = (xmlValidCtxtPyCtxtPtr)xmlMalloc(sizeof(xmlValidCtxtPyCtxt));
    if (pyCtxt == NULL) {
        py_retval = libxml_intWrap(-1);
        return(py_retval);
    }
    memset(pyCtxt, 0, sizeof(xmlValidCtxtPyCtxt));

    /* Reference handling (no-ops in this stub) */
    Py_XDECREF(pyCtxt->error);
    Py_XINCREF(pyobj_error);
    pyCtxt->error = pyobj_error;

    Py_XDECREF(pyCtxt->warn);
    Py_XINCREF(pyobj_warn);
    pyCtxt->warn = pyobj_warn;

    Py_XDECREF(pyCtxt->arg);
    Py_XINCREF(pyobj_arg);
    pyCtxt->arg = pyobj_arg;

    /* Null-pointer dereference happens right here when ctxt == NULL */
    ctxt->error = libxml_xmlValidCtxtErrorFuncHandler;     /* boom */
    ctxt->warning = libxml_xmlValidCtxtWarningFuncHandler;
    ctxt->userData = pyCtxt;

    py_retval = libxml_intWrap(1);
    return (py_retval);
}

int main(void)
{
    /* Prepare fake Python objects for ctx, error, warn, arg */
    PyObject fake_ctx = { .tag = 0 };   /* invalid validator context */
    PyObject fake_error = { .tag = 1 };
    PyObject fake_warn = { .tag = 2 };
    PyObject fake_arg = { .tag = 3 };

    /* Package them into a tuple-like container for our PyArg_ParseTuple stub */
    PyTupleArgs args;
    memset(&args, 0, sizeof(args));
    args.items[0] = &fake_ctx;   /* pyobj_ctx */
    args.items[1] = &fake_error; /* pyobj_error */
    args.items[2] = &fake_warn;  /* pyobj_warn */
    args.items[3] = &fake_arg;   /* pyobj_arg (optional) */
    args.count = 4;

    /* Call the vulnerable function: this will dereference NULL (ctxt) */
    (void)libxml_xmlSetValidErrors(NULL, (PyObject *)&args);

    /* If the bug didn't trigger (it should), exit non-zero to signal failure */
    fprintf(stderr, "Expected NULL dereference did not occur.\n");
    return 1;
}
