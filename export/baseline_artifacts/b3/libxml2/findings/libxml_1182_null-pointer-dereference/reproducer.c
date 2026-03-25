#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

/* Minimal stand-ins for libxml2/Python types */
typedef unsigned char xmlChar;

typedef struct {
    int dummy;
} PyObject;

/* Stubs for the Python C-API used by python/libxml.c */
int PyObject_HasAttrString(PyObject *obj, const char *name) {
    /* Force the vulnerable call path to be taken */
    (void)obj; (void)name;
    return 1; /* Pretend the handler has attribute "notationDecl" */
}

/* Dummy decref to satisfy Py_XDECREF */
void Py_DecRef(PyObject *op) { (void)op; }
#define Py_XDECREF(op) do { if ((op) != NULL) { Py_DecRef((op)); (op) = NULL; } } while (0)

int PyErr_Occurred(void) { return 0; }
void PyErr_Print(void) { }

/*
 * Stub for PyObject_CallMethod that simulates the behavior relevant to the bug:
 * the "s" format does NOT accept NULL. CPython would dereference the NULL when
 * trying to create a Python str from it. We simulate that by dereferencing the
 * pointer passed for each 's' argument.
 */
PyObject *PyObject_CallMethod(PyObject *obj, const char *name, const char *format, ...) {
    (void)obj; (void)name;
    va_list ap;
    va_start(ap, format);
    for (const char *p = format; p && *p; p++) {
        switch (*p) {
            case 's': {
                const char *s = va_arg(ap, const char *);
                /* Simulate CPython "s" conversion: NULL is invalid and will crash */
                volatile char c = s[0]; /* Will NULL-deref if s == NULL */
                (void)c;
                break;
            }
            case 'i':
            case 'l':
                (void)va_arg(ap, int);
                break;
            case 'd':
                (void)va_arg(ap, double);
                break;
            case 'O':
            default:
                (void)va_arg(ap, void *);
                break;
        }
    }
    va_end(ap);
    return NULL;
}

/*
 * Direct transcription of the vulnerable function from python/libxml.c
 */
static void
pythonNotationDecl(void *user_data,
                   const xmlChar *name,
                   const xmlChar *publicId,
                   const xmlChar *systemId)
{
    PyObject *handler;
    PyObject *result;

    handler = (PyObject *) user_data;
    if (PyObject_HasAttrString(handler, "notationDecl")) {
        result = PyObject_CallMethod(handler, "notationDecl",
                                     "sss", (const char *)name, (const char *)publicId,
                                     (const char *)systemId);
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(result);
    }
}

int main(void) {
    /* Craft inputs: name non-NULL, publicId or systemId NULL as allowed by libxml2 */
    const xmlChar *name = (const xmlChar *)"ExampleNotation";
    const xmlChar *publicId = NULL;   /* This is permitted by libxml2 */
    const xmlChar *systemId = NULL;   /* This is permitted by libxml2 */

    PyObject handler = {0};

    /* Trigger the vulnerable path: format "sss" with NULL publicId/systemId */
    pythonNotationDecl(&handler, name, publicId, systemId);

    /* We should have crashed before reaching here due to NULL dereference */
    puts("If you see this, the reproduction did not trigger.");
    return 0;
}
