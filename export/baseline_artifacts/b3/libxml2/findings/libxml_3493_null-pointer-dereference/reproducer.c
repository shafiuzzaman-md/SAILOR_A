#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Minimal stubs to mimic the Python C API used by the vulnerable code */
#define ATTRIBUTE_UNUSED

typedef struct FakePyObject {
    int kind;
} PyObject;

/* Stub: pretend argument parsing succeeded and return the provided pointer */
int PyArg_ParseTuple(PyObject *args, const char *fmt, PyObject **out) {
    (void)fmt; /* unused */
    *out = (PyObject *)args; /* Treat args as the single object argument */
    return 1; /* success */
}

/* Stub: return some name (non-NULL) to simulate a capsule name lookup */
const char *PyCapsule_GetName(PyObject *obj) {
    (void)obj;
    return "any_name"; /* non-NULL */
}

/* Stub: always fail to get the pointer from the capsule (returns NULL) */
void *PyCapsule_GetPointer(PyObject *obj, const char *name) {
    (void)obj;
    (void)name;
    return NULL; /* Simulate failure per vulnerability description */
}

/* Stub: emulate Py_BuildValue("s", str). This will deref NULL via strlen */
PyObject *Py_BuildValue(const char *fmt, const char *str) {
    if (fmt && fmt[0] == 's') {
        /* Vulnerable behavior: assume str is non-NULL and use it */
        size_t n = strlen(str); /* NULL deref if str == NULL */
        /* Allocate a dummy PyObject to satisfy return type (not reached) */
        PyObject *o = (PyObject *)malloc(sizeof(PyObject));
        if (o) o->kind = (int)n;
        return o;
    }
    return NULL;
}

/* Vulnerable function from python/libxml.c (adapted to use our stubs) */
static PyObject *
libxml_getObjDesc(PyObject *self ATTRIBUTE_UNUSED, PyObject *args) {
    PyObject *obj;
    char *str;

    if (!PyArg_ParseTuple(args, "O:getObjDesc", &obj))
        return NULL;
    str = (char *)PyCapsule_GetPointer(obj, PyCapsule_GetName(obj));
    return Py_BuildValue("s", str);
}

int main(void) {
    /* Craft a non-capsule object; our stubs will make GetPointer return NULL */
    PyObject fake_obj;
    fake_obj.kind = 1234;

    /* Call the vulnerable function: it will end up calling strlen(NULL) */
    (void)libxml_getObjDesc(NULL, &fake_obj);

    /* If the program did not crash (it should), indicate abnormal behavior */
    fprintf(stderr, "Unexpectedly did not crash.\n");
    return 0;
}
