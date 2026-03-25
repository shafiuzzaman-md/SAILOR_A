#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdint.h>

/* Minimal stand-ins for Python C-API types and functions */
typedef struct { int dummy; } PyObject;

static int PyObject_HasAttrString(PyObject *obj, const char *name) {
    /* Always claim the handler has the attribute so the vulnerable path is taken */
    (void)obj; (void)name;
    return 1;
}

static int PyErr_Occurred(void) {
    return 0;
}

static void PyErr_Print(void) {
}

#define Py_XDECREF(o) do { (void)(o); } while (0)

/*
 * Stub that mimics the dangerous behavior of CPython's PyObject_CallMethod
 * when using the "s" format with a NULL char* argument. Here we explicitly
 * dereference each 's' argument to trigger a NULL pointer dereference.
 */
static PyObject *PyObject_CallMethod(PyObject *self, const char *method, const char *fmt, ...) {
    (void)self; (void)method;
    va_list ap;
    va_start(ap, fmt);
    for (const char *p = fmt; *p; ++p) {
        if (*p == 's') {
            const char *s = va_arg(ap, const char *);
            /* This will crash if s == NULL, mirroring CPython's behavior for 's' */
            volatile char c = s[0];
            (void)c;
        } else if (*p == 'i') {
            (void)va_arg(ap, int);
        } else if (*p == '#') {
            /* length specifier paired with a previous 's' (not used in this path) */
            (void)va_arg(ap, int);
        } else {
            /* Consume as pointer for any other unexpected specifier to keep varargs aligned. */
            (void)va_arg(ap, void *);
        }
    }
    va_end(ap);
    /* Return non-NULL dummy object */
    return (PyObject *)0x1;
}

/* Minimal libxml2-like typedef */
typedef unsigned char xmlChar;

/* Vulnerable function copied and minimized from python/libxml.c */
static void pythonEntityDecl(void *user_data,
                             const xmlChar * name,
                             int type,
                             const xmlChar * publicId,
                             const xmlChar * systemId,
                             xmlChar * content) {
    PyObject *handler;
    PyObject *result;

    handler = (PyObject *) user_data;
    if (PyObject_HasAttrString(handler, "entityDecl")) {
        /* Vulnerable call: uses 's' for args that can be NULL (publicId/systemId/content) */
        result = PyObject_CallMethod(handler, "entityDecl",
                                     "sisss", name, type,
                                     (const char *)publicId, (const char *)systemId, (const char *)content);
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(result);
    }
}

int main(void) {
    PyObject handler = {0};

    const xmlChar *name = (const xmlChar *)"extEntity";
    /* Use a value representing an external entity; the exact value is irrelevant for the crash. */
    int type = 2;

    /* For external entities, content is NULL in libxml2. publicId/systemId may also be NULL. */
    const xmlChar *publicId = (const xmlChar *)"PUBLIC-ID";
    const xmlChar *systemId = (const xmlChar *)"http://example.com/ext.ent";
    xmlChar *content = NULL;  /* Critical: NULL passed to 's' in format string */

    /* This call will cause our PyObject_CallMethod stub to dereference NULL */
    pythonEntityDecl(&handler, name, type, publicId, systemId, content);

    /* Not reached */
    return 0;
}
