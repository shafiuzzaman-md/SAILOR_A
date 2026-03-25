#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

/* Minimal stand-ins for types used by python/libxml.c */
typedef unsigned char xmlChar;

typedef struct _PyObject {
    int dummy;
} PyObject;

/* Stubs for the small subset of the Python C-API used */
int PyObject_HasAttrString(PyObject *obj, const char *name) {
    /* Pretend the handler implements only processingInstruction */
    if (name && strcmp(name, "processingInstruction") == 0)
        return 1;
    return 0;
}

/* Py_XDECREF is a no-op for our stub environment */
#define Py_XDECREF(op) ((void)0)

/*
 * Stub for PyObject_CallMethod that mimics the dangerous behavior of the
 * "s" formatter: passing NULL for an "s" argument leads to a NULL deref
 * when the C-API tries to use the char*.
 */
PyObject *PyObject_CallMethod(PyObject *obj, const char *name, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);

    /* We only care about the vulnerable path: format "ss" */
    if (fmt && strcmp(fmt, "ss") == 0) {
        const char *arg1 = va_arg(ap, const char *);
        const char *arg2 = va_arg(ap, const char *);
        (void)arg1;
        /* Simulate CPython expecting a non-NULL C string for format 's'. */
        /* This will crash with a NULL pointer dereference if arg2 == NULL. */
        size_t len2 = strlen(arg2);  /* Intentional NULL dereference when arg2 == NULL */
        (void)len2;
    }

    va_end(ap);
    return (PyObject *)obj; /* Return a non-NULL dummy */
}

/* === Code under test (mirrors python/libxml.c snippet) === */
static void pythonProcessingInstruction(void *user_data,
                                        const xmlChar *target,
                                        const xmlChar *data) {
    PyObject *handler;
    PyObject *result;

    handler = (PyObject *) user_data;
    if (PyObject_HasAttrString(handler, "processingInstruction")) {
        /* Vulnerable call: format "ss" with potential NULL for data */
        result = PyObject_CallMethod(handler, "processingInstruction", "ss",
                                     (const char *)target, (const char *)data);
        Py_XDECREF(result);
    }
}

int main(void) {
    /* Set up a fake Python handler object */
    PyObject handler = {0};

    /* target is a normal string, data is NULL to simulate PI without data */
    const xmlChar *target = (const xmlChar *)"pi-target";
    const xmlChar *data = NULL;  /* This triggers the crash in the stub */

    /* This call will reach PyObject_CallMethod with format "ss" and NULL */
    pythonProcessingInstruction((void *)&handler, target, data);

    /* Should not reach here; if it does, indicate failure */
    fprintf(stderr, "Did not crash as expected.\n");
    return 0;
}
