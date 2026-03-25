#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>

/* Minimal typedef to match libxml2 */
typedef unsigned char xmlChar;

/* Minimal stub for a CPython PyObject */
typedef struct {
    int has_externalSubset;
} PyObject;

/* Stubs for the limited CPython C-API used by the vulnerable code */
static int PyObject_HasAttrString(PyObject *obj, const char *attr) {
    if (obj == NULL || attr == NULL) return 0;
    /* Pretend our handler has only the 'externalSubset' attribute when flagged */
    if (strcmp(attr, "externalSubset") == 0)
        return obj->has_externalSubset ? 1 : 0;
    return 0;
}

/*
 * Stub that mimics the unsafe handling of format string "sss".
 * For each 's', it treats the argument as a C string and dereferences it
 * (via strlen) which will crash if NULL is passed, reproducing the bug.
 */
static PyObject *PyObject_CallMethod(PyObject *self, const char *method, const char *fmt, ...) {
    (void)self;
    (void)method;
    va_list ap;
    va_start(ap, fmt);

    for (const char *p = fmt; p && *p; p++) {
        if (*p == 's') {
            const char *s = va_arg(ap, const char *);
            /* This strlen will dereference NULL if s is NULL, reproducing the bug. */
            size_t len = strlen(s);
            /* Do something with it to avoid optimizing away */
            char *tmp = (char *)malloc(len + 1);
            if (tmp) {
                memcpy(tmp, s, len + 1);
                free(tmp);
            }
        } else if (*p == '#') {
            /* Not used in this path; consume an int if it appears */
            (void)va_arg(ap, int);
        } else if (*p == 'i') {
            (void)va_arg(ap, int);
        } else if (*p == 'l') {
            (void)va_arg(ap, long);
        } else if (*p == 'd' || *p == 'f') {
            (void)va_arg(ap, double);
        } else {
            /* Consume as pointer for unknown specifiers to keep va_list in sync */
            (void)va_arg(ap, void *);
        }
    }

    va_end(ap);
    return NULL;
}

#define Py_XDECREF(obj) ((void)0)

/* Vulnerable function from python/libxml.c reproduced here */
static void pythonExternalSubset(void *user_data,
                                 const xmlChar *name,
                                 const xmlChar *externalID,
                                 const xmlChar *systemID) {
    PyObject *handler;
    PyObject *result;

    handler = (PyObject *)user_data;
    if (PyObject_HasAttrString(handler, "externalSubset")) {
        /* externalID and/or systemID can be NULL; with format "sss" this crashes */
        result = PyObject_CallMethod(handler, "externalSubset", "sss",
                                     name, externalID, systemID);
        Py_XDECREF(result);
    }
}

int main(void) {
    /* Set up a handler that advertises an externalSubset method */
    PyObject handler;
    handler.has_externalSubset = 1;

    const xmlChar *name = (const xmlChar *)"doc";   /* non-NULL */
    const xmlChar *externalID = NULL;                /* will trigger the NULL deref */
    const xmlChar *systemID = NULL;                  /* also NULL */

    /* Directly invoke the vulnerable callback with NULL IDs */
    pythonExternalSubset(&handler, name, externalID, systemID);

    /* We should never reach here due to crash */
    printf("If you see this, the crash did not occur as expected.\n");
    return 0;
}
