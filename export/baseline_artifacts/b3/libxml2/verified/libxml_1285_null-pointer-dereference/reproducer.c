#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdint.h>

/* Minimal typedef to match libxml2's xmlChar */
typedef unsigned char xmlChar;

/* Minimal stand-ins for Python C-API objects and functions */
typedef struct { int dummy; } PyObject;

/* Simulate attribute presence: report that handler has "internalSubset" */
int PyObject_HasAttrString(PyObject *handler, const char *attr) {
    (void)handler;
    /* For the purpose of the reproducer, pretend the attribute exists */
    if (attr && strcmp(attr, "internalSubset") == 0)
        return 1;
    return 0;
}

/* Simulate PyErr_Occurred / PyErr_Print (no-op for this reproducer) */
int PyErr_Occurred(void) { return 0; }
void PyErr_Print(void) { }

/* Simulate Py_XDECREF by freeing allocated stub objects */
void Py_XDECREF(PyObject *obj) {
    if (obj) free(obj);
}

/*
 * Stub for PyObject_CallMethod that purposefully mimics the dangerous behavior
 * of the Python C-API "s" format: it dereferences the provided char*.
 * Using strlen() on a NULL pointer will cause a NULL dereference under ASan.
 */
PyObject *PyObject_CallMethod(PyObject *handler, const char *name, const char *format, ...) {
    (void)handler;
    (void)name;

    va_list ap;
    va_start(ap, format);

    for (const char *f = format; f && *f; ++f) {
        switch (*f) {
            case 's': {
                /* Expect a char* argument; emulate Python's behavior of requiring non-NULL */
                const char *s = va_arg(ap, const char *);
                /* This strlen will crash if s == NULL, which is what we want to expose */
                size_t len = strlen(s);
                /* Do something with len to avoid optimizing away */
                if (len == (size_t)-1) {
                    fprintf(stderr, "Impossible branch\n");
                }
                break;
            }
            case 'i': {
                (void)va_arg(ap, int);
                break;
            }
            case 'O': {
                (void)va_arg(ap, void *);
                break;
            }
            default:
                /* Ignore other specifiers for this stub */
                break;
        }
    }

    va_end(ap);

    /* Return a dummy PyObject */
    PyObject *ret = (PyObject *)malloc(sizeof(PyObject));
    if (ret) ret->dummy = 0;
    return ret;
}

/* The vulnerable function as in python/libxml.c */
static void pythonInternalSubset(void *user_data, const xmlChar *name,
                                 const xmlChar *ExternalID, const xmlChar *SystemID) {
    PyObject *handler;
    PyObject *result;

    handler = (PyObject *)user_data;
    if (PyObject_HasAttrString(handler, "internalSubset")) {
        /* BUG: ExternalID or SystemID may be NULL, but format uses "s" */
        result = PyObject_CallMethod(handler, "internalSubset", "sss",
                                     (const char *)name,
                                     (const char *)ExternalID,
                                     (const char *)SystemID);
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(result);
    }
}

int main(void) {
    /* Prepare a dummy handler object */
    PyObject handler = {0};

    /* Name is valid, ExternalID is intentionally NULL, SystemID is valid */
    const xmlChar *name = (const xmlChar *)"doc";
    const xmlChar *ExternalID = NULL;  /* This should trigger the NULL deref in "s" */
    const xmlChar *SystemID = (const xmlChar *)"http://example.com/system-id";

    fprintf(stderr, "About to trigger pythonInternalSubset with NULL ExternalID...\n");

    /* Call the vulnerable path */
    pythonInternalSubset((void *)&handler, name, ExternalID, SystemID);

    /* If we got here without crashing, something went wrong with the reproducer */
    fprintf(stderr, "Reproducer did not crash as expected.\n");
    return 0;
}