#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

/* Minimal stand-ins for libxml2/python types */
typedef char xmlChar;

typedef struct { int dummy; } PyObject;

/* Stubs for the small subset of Python C-API that the vulnerable code uses */
static int PyObject_HasAttrString(PyObject *obj, const char *attr) {
    (void)obj;
    /* Pretend the handler implements the method we want to call */
    return (attr && strcmp(attr, "unparsedEntityDecl") == 0) ? 1 : 0;
}

/* Simulate CPython's PyObject_CallMethod with "s" format behavior.
 * The real CPython API treats "s" as a C string and will crash if given NULL.
 * Here we explicitly dereference the string to mimic that crash. */
static PyObject *PyObject_CallMethod(PyObject *obj, const char *method, const char *format, ...) {
    (void)obj;
    (void)method;
    va_list ap;
    va_start(ap, format);
    for (const char *p = format; *p; p++) {
        if (*p == 's') {
            const char *s = va_arg(ap, const char *);
            /* This will crash (null-pointer-dereference) if s == NULL */
            volatile char c = s[0];
            (void)c;
        } else if (*p == 'i') {
            (void)va_arg(ap, int);
        } else if (*p == 'O') {
            (void)va_arg(ap, void *);
        } else {
            /* Consume unknown specifiers conservatively as pointers */
            (void)va_arg(ap, void *);
        }
    }
    va_end(ap);
    /* Return a dummy object; caller will decref */
    return (PyObject *)malloc(sizeof(PyObject));
}

static int PyErr_Occurred(void) { return 0; }
static void PyErr_Print(void) { fprintf(stderr, "PyErr_Print called\n"); }
#define Py_XDECREF(obj) do { if ((obj) != NULL) free(obj); } while (0)

/* Vulnerable function from python/libxml.c, reduced to needed parts */
static void pythonUnparsedEntityDecl(void *user_data,
                                     const xmlChar *name,
                                     const xmlChar *publicId,
                                     const xmlChar *systemId,
                                     const xmlChar *notationName) {
    PyObject *handler;
    PyObject *result;

    handler = (PyObject *)user_data;
    if (PyObject_HasAttrString(handler, "unparsedEntityDecl")) {
        result = PyObject_CallMethod(handler, "unparsedEntityDecl", "ssss",
                                     (const char *)name,
                                     (const char *)publicId,      /* may be NULL: triggers crash */
                                     (const char *)systemId,
                                     (const char *)notationName);
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(result);
    }
}

int main(void) {
    PyObject handler; /* dummy handler object */

    const xmlChar *name = (const xmlChar *)"entityName";
    const xmlChar *publicId = NULL; /* libxml2 may pass NULL here */
    const xmlChar *systemId = (const xmlChar *)"http://example.com/systemId";
    const xmlChar *notationName = (const xmlChar *)"notation";

    fprintf(stderr, "About to call pythonUnparsedEntityDecl with NULL publicId...\n");
    /* This call will lead to NULL dereference inside PyObject_CallMethod stub
       because format = "ssss" and one of the 's' arguments is NULL. */
    pythonUnparsedEntityDecl(&handler, name, publicId, systemId, notationName);

    /* If the process didn't crash, something went wrong with the reproducer */
    fprintf(stderr, "Unexpectedly survived the vulnerable call.\n");
    return 0;
}
