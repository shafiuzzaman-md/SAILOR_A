#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Minimal type stubs to mirror the libxml Python binding environment */
typedef unsigned char xmlChar;
typedef void* xmlXPathContextPtr;

typedef struct _PyObject {
    int refcnt;
} PyObject;

/* Callback entry as used by the Python binding */
typedef struct {
    xmlXPathContextPtr ctx;
    xmlChar *name;
    xmlChar *ns_uri;
    PyObject *function;
} xmlXPathCallback;

typedef xmlXPathCallback* xmlXPathCallbackArray;

/* Globals matching the binding's storage */
static xmlXPathCallbackArray* libxml_xpathCallbacks = NULL;
static int libxml_xpathCallbacksAllocd = 0;
static int libxml_xpathCallbacksNb = 0;
static int libxml_xpathCallbacksInitialized = 0;

/* Stubs for Python/macros */
#define Py_XINCREF(op) do { if ((op) != NULL) ((PyObject*)(op))->refcnt++; } while (0)
#define Py_XDECREF(op) do { if ((op) != NULL) ((PyObject*)(op))->refcnt--; } while (0)

static PyObject* libxml_intWrap(int v) {
    (void)v; /* not used in crash path */
    PyObject *o = (PyObject*)calloc(1, sizeof(PyObject));
    return o;
}

/* libxml2 API stubs used in the vulnerable function */
static void libxml_xpathCallbacksInitialize(void) {
    libxml_xpathCallbacksInitialized = 1;
}

static void libxml_xmlXPathFuncLookupFunc(void) { /* dummy */ }

static void xmlXPathRegisterFuncLookup(xmlXPathContextPtr ctx, void* f, void* data) {
    (void)ctx; (void)f; (void)data; /* no-op */
}

/* xmlStr helpers */
static int xmlStrEqual(const xmlChar *a, const xmlChar *b) {
    if (a == NULL || b == NULL) return a == b;
    return strcmp((const char*)a, (const char*)b) == 0;
}

static xmlChar* xmlStrdup(const xmlChar *s) {
    if (!s) return NULL;
    size_t n = strlen((const char*)s) + 1;
    xmlChar *out = (xmlChar*)malloc(n);
    if (out) memcpy(out, s, n);
    return out;
}

/* Critical: xmlRealloc stub that simulates allocation failure (returns NULL) */
static void* xmlRealloc(void* ptr, size_t size) {
    (void)ptr; (void)size;
    /* Simulate OOM to trigger the bug path */
    return NULL;
}

/* Reimplementation of the vulnerable function, simplified to directly set up inputs. */
PyObject * libxml_xmlRegisterXPathFunction(PyObject * self, PyObject * args) {
    (void)self; (void)args;

    PyObject *py_retval;
    int c_retval = 0;
    xmlChar *name = (xmlChar*)"myfunc";
    xmlChar *ns_uri = (xmlChar*)"urn:ns";
    xmlXPathContextPtr ctx = (xmlXPathContextPtr)0x1234; /* dummy non-NULL */
    PyObject *pyobj_ctx = (PyObject*)0x1;                 /* dummy non-NULL to avoid early return */
    PyObject *pyobj_f = (PyObject*)calloc(1, sizeof(PyObject)); /* function object */
    int i;

    if (libxml_xpathCallbacksInitialized == 0)
        libxml_xpathCallbacksInitialize();
    xmlXPathRegisterFuncLookup(ctx, (void*)libxml_xmlXPathFuncLookupFunc, ctx);

    if ((pyobj_ctx == NULL) || (name == NULL) || (pyobj_f == NULL)) {
        py_retval = libxml_intWrap(-1);
        return (py_retval);
    }

    /* No existing callbacks yet, skip the loop effectively */
    for (i = 0; i < libxml_xpathCallbacksNb; i++) {
        if ((ctx == (*libxml_xpathCallbacks)[i].ctx) &&
            (xmlStrEqual(name, (*libxml_xpathCallbacks)[i].name)) &&
            (xmlStrEqual(ns_uri, (*libxml_xpathCallbacks)[i].ns_uri))) {
            Py_XINCREF(pyobj_f);
            Py_XDECREF((*libxml_xpathCallbacks)[i].function);
            (*libxml_xpathCallbacks)[i].function = pyobj_f;
            c_retval = 1;
            goto done;
        }
    }

    /* Force reallocation path; our xmlRealloc returns NULL simulating OOM */
    if (libxml_xpathCallbacksNb >= libxml_xpathCallbacksAllocd) {
        libxml_xpathCallbacksAllocd += 10;
        libxml_xpathCallbacks = (xmlXPathCallbackArray*)xmlRealloc(
            libxml_xpathCallbacks,
            (size_t)libxml_xpathCallbacksAllocd * sizeof(xmlXPathCallback));
    }

    /* Following lines dereference libxml_xpathCallbacks without NULL check */
    i = libxml_xpathCallbacksNb++;
    Py_XINCREF(pyobj_f);

    /* This write will dereference a NULL base pointer (libxml_xpathCallbacks) */
    (*libxml_xpathCallbacks)[i].ctx = ctx;               /* NULL deref here */
    (*libxml_xpathCallbacks)[i].name = xmlStrdup(name);
    (*libxml_xpathCallbacks)[i].ns_uri = xmlStrdup(ns_uri);
    (*libxml_xpathCallbacks)[i].function = pyobj_f;
    c_retval = 1;

  done:
    py_retval = libxml_intWrap((int) c_retval);
    return (py_retval);
}

int main(void) {
    /* Call the vulnerable function; it will crash due to NULL dereference */
    (void)libxml_xmlRegisterXPathFunction(NULL, NULL);
    /* If it somehow returns, print something (should not happen) */
    puts("Unexpectedly returned");
    return 0;
}
