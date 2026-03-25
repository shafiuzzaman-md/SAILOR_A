#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

/* Minimal stand-ins for Python C-API types/macros used by the vulnerable code */
typedef struct { int dummy; } PyObject;

#define Py_XDECREF(o) ((void)(o))
#define Py_XINCREF(o) ((void)(o))
#define Py_INCREF(o)  ((void)(o))

static PyObject g_PyNoneStruct;
PyObject *Py_None = &g_PyNoneStruct;

/* Global to let our PyArg_ParseTuple stub return a crafted object */
static PyObject *g_pyobj_cur = NULL;

/* Minimal stand-ins for libxml2 validator context types used by the vulnerable code */
typedef struct _xmlValidCtxt {
    void *userData; /* first field, as accessed by the vulnerable code */
} xmlValidCtxt, *xmlValidCtxtPtr;

typedef struct {
    PyObject *error;
    PyObject *warn;
    PyObject *arg;
} xmlValidCtxtPyCtxt, *xmlValidCtxtPyCtxtPtr;

/* Stubs for libxml2 memory funcs used by the vulnerable code */
void xmlFree(void *ptr) { free(ptr); }
void xmlFreeValidCtxt(xmlValidCtxtPtr cur) { free(cur); }

/* Stub for Python argument parsing used by the vulnerable wrapper */
int PyArg_ParseTuple(PyObject *args, const char *format, ...) {
    (void)args;
    (void)format; /* format is "O:xmlFreeValidCtxt" in the real code */
    va_list ap;
    va_start(ap, format);
    /* The vulnerable function expects to receive the address of a PyObject* output param */
    PyObject **out_obj = va_arg(ap, PyObject **);
    *out_obj = g_pyobj_cur; /* Hand back our crafted object */
    va_end(ap);
    return 1; /* success */
}

/* Stub for PyValidCtxt_Get: return NULL to simulate an invalid/closed validator context */
void *PyValidCtxt_Get(PyObject *obj) {
    (void)obj;
    return NULL; /* This is the crux: makes cur == NULL in the vulnerable code */
}

/* Vulnerable function body reproduced with the minimal surrounding context */
PyObject *libxml_xmlFreeValidCtxt(PyObject *self, PyObject *args) {
    (void)self; /* ATTRIBUTE_UNUSED in the original */
    xmlValidCtxtPtr cur;
    xmlValidCtxtPyCtxtPtr pyCtxt;
    PyObject *pyobj_cur;

    if (!PyArg_ParseTuple(args, "O:xmlFreeValidCtxt", &pyobj_cur))
        return NULL;
    cur = (xmlValidCtxtPtr) PyValidCtxt_Get(pyobj_cur);

    /* NULL pointer dereference: cur can be NULL when PyValidCtxt_Get fails */
    pyCtxt = (xmlValidCtxtPyCtxtPtr)(cur->userData);

    if (pyCtxt != NULL) {
        Py_XDECREF(pyCtxt->error);
        Py_XDECREF(pyCtxt->warn);
        Py_XDECREF(pyCtxt->arg);
        xmlFree(pyCtxt);
    }

    xmlFreeValidCtxt(cur);
    Py_INCREF(Py_None);
    return Py_None;
}

int main(void) {
    /* Craft an arbitrary PyObject that represents an invalid/closed validator */
    static PyObject invalid_validator_obj; /* contents irrelevant for this repro */
    g_pyobj_cur = &invalid_validator_obj;

    /* Dummy args object; our PyArg_ParseTuple stub ignores it and returns g_pyobj_cur */
    PyObject dummy_args;

    /* This call will crash with a NULL-pointer dereference at cur->userData */
    (void)libxml_xmlFreeValidCtxt(NULL, &dummy_args);

    /* Not reached */
    return 0;
}
