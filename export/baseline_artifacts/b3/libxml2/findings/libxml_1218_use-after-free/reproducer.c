// Standalone C reproducer for use-after-free in pythonAttributeDecl
// Compile with:
//   clang -fsanitize=address -g -O0 reproducer.c -o reproducer

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

// Minimal stand-ins for libxml2 types used by the vulnerable function
typedef unsigned char xmlChar;

typedef struct _xmlEnumeration {
    const xmlChar *name;
    struct _xmlEnumeration *next;
} xmlEnumeration, *xmlEnumerationPtr;

// Minimal stand-ins for CPython objects and API to model refcounting semantics

typedef enum {
    OBJ_STRING = 1,
    OBJ_LIST   = 2,
    OBJ_HANDLER= 3
} ObjType;

typedef struct _PyObject PyObject;

struct _PyObject {
    int refcnt;
    ObjType type;
    union {
        struct { char *s; } str;
        struct { PyObject **items; int size; } list;
        struct { int dummy; } handler;
    } as;
};

// Forward declarations
static void Py_DECREF(PyObject *o);

// Helpers to simulate CPython API
static PyObject *py_string_new(const char *s) {
    PyObject *o = (PyObject *)malloc(sizeof(PyObject));
    if (!o) abort();
    o->refcnt = 1;
    o->type = OBJ_STRING;
    o->as.str.s = strdup(s ? s : "");
    return o;
}

static PyObject *PyList_New(int n) {
    PyObject *o = (PyObject *)malloc(sizeof(PyObject));
    if (!o) abort();
    o->refcnt = 1;
    o->type = OBJ_LIST;
    o->as.list.size = n;
    o->as.list.items = (PyObject **)calloc((size_t)n, sizeof(PyObject *));
    return o;
}

// PyList_SetItem steals a reference to 'item' (no INCREF)
static void PyList_SetItem(PyObject *list, int index, PyObject *item) {
    if (!list || list->type != OBJ_LIST) return;
    if (index < 0 || index >= list->as.list.size) return;
    list->as.list.items[index] = item; // Steals reference
}

// Minimal handler object
static PyObject *new_handler(void) {
    PyObject *o = (PyObject *)malloc(sizeof(PyObject));
    if (!o) abort();
    o->refcnt = 1;
    o->type = OBJ_HANDLER;
    o->as.handler.dummy = 0;
    return o;
}

static int PyObject_HasAttrString(PyObject *obj, const char *attr) {
    if (!obj) return 0;
    if (obj->type != OBJ_HANDLER) return 0;
    return (strcmp(attr, "attributeDecl") == 0) ? 1 : 0;
}

static int PyErr_Occurred(void) { return 0; }
static void PyErr_Print(void) { }

// Very small stub that pretends to call a Python method; does not touch list items
static PyObject *PyObject_CallMethod(PyObject *o, const char *name, const char *fmt, ...) {
    (void)o; (void)name; (void)fmt;
    // Just return a new dummy object to mirror a typical non-NULL return
    PyObject *ret = (PyObject *)malloc(sizeof(PyObject));
    if (!ret) abort();
    ret->refcnt = 1;
    ret->type = OBJ_HANDLER; // arbitrary
    ret->as.handler.dummy = 42;
    return ret;
}

#define PY_IMPORT_STRING(x) py_string_new((const char *)(x))
#define Py_XDECREF(o) do { if ((o) != NULL) Py_DECREF((o)); } while (0)
#define XML_IGNORE_DEPRECATION_WARNINGS
#define XML_POP_WARNINGS

static void Py_DECREF(PyObject *o) {
    if (!o) return;
    // This access will trigger UAF if 'o' points to freed memory
    o->refcnt--;
    if (o->refcnt == 0) {
        switch (o->type) {
            case OBJ_STRING:
                free(o->as.str.s);
                break;
            case OBJ_LIST: {
                // DECREF all items held by the list (will touch freed memory if items are dangling)
                for (int i = 0; i < o->as.list.size; i++) {
                    PyObject *it = o->as.list.items[i];
                    if (it) {
                        Py_DECREF(it); // Potential UAF here
                    }
                }
                free(o->as.list.items);
                break;
            }
            case OBJ_HANDLER:
                // nothing
                break;
            default:
                break;
        }
        free(o);
    }
}

// Vulnerable function ported from python/libxml.c
static void
pythonAttributeDecl(void *user_data,
                    const xmlChar * elem,
                    const xmlChar * name,
                    int type,
                    int def,
                    const xmlChar * defaultValue, xmlEnumerationPtr tree)
{
    PyObject *handler;
    PyObject *nameList;
    PyObject *newName;
    xmlEnumerationPtr node;
    PyObject *result;
    int count;

XML_IGNORE_DEPRECATION_WARNINGS
    handler = (PyObject *) user_data;
    if (PyObject_HasAttrString(handler, "attributeDecl")) {
        count = 0;
        for (node = tree; node != NULL; node = node->next) {
            count++;
        }
        nameList = PyList_New(count);
        count = 0;
        for (node = tree; node != NULL; node = node->next) {
            newName = PY_IMPORT_STRING((char *) node->name);
            PyList_SetItem(nameList, count, newName);
            // BUG: PyList_SetItem steals reference; this extra DECREF frees newName prematurely
            Py_DECREF(newName);
            count++;
        }
        result = PyObject_CallMethod(handler, "attributeDecl",
                                     "ssiisO", elem, name, type,
                                     def, defaultValue, nameList);
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(nameList);   // Will traverse list and DECREF freed items -> UAF
        Py_XDECREF(result);
    }
XML_POP_WARNINGS
}

int main(void) {
    // Build a small xmlEnumeration list with two names
    xmlEnumeration *n1 = (xmlEnumeration *)malloc(sizeof(xmlEnumeration));
    xmlEnumeration *n2 = (xmlEnumeration *)malloc(sizeof(xmlEnumeration));
    if (!n1 || !n2) abort();
    n1->name = (const xmlChar *)"ONE";
    n1->next = n2;
    n2->name = (const xmlChar *)"TWO";
    n2->next = NULL;

    // Create a handler object that advertises the attributeDecl method
    PyObject *handler = new_handler();

    // Call the vulnerable function. The UAF will occur when nameList is DECREF'd.
    pythonAttributeDecl((void *)handler,
                        (const xmlChar *)"elem",
                        (const xmlChar *)"attr",
                        0, 0,
                        (const xmlChar *)NULL,
                        n1);

    // Cleanup remaining references
    Py_XDECREF(handler);

    // Free enumeration nodes (unrelated to the UAF)
    free(n2);
    free(n1);

    // Prevent compiler from optimizing too aggressively
    puts("Done. If running under ASan, a use-after-free should have been reported above.");
    return 0;
}
