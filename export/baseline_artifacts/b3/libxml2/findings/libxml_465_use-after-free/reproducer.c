#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Minimal stand-ins for libxml2 types */
typedef void *xmlCharEncodingHandlerPtr;

typedef int (*xmlOutputWriteCallback)(void * context, const char * buffer, int len);
typedef int (*xmlOutputCloseCallback)(void * context);

typedef struct _xmlOutputBuffer {
    xmlOutputWriteCallback writecallback;
    xmlOutputCloseCallback closecallback;
    void *context;
    xmlCharEncodingHandlerPtr encoder;
} xmlOutputBuffer, *xmlOutputBufferPtr;

/* Minimal stand-ins for Python C-API objects and functions */
typedef struct _PyObject {
    int refcnt;
    int has_io_close;
    int has_flush;
    int has_write;
    int magic; /* arbitrary payload to touch */
} PyObject;

static void Py_DECREF(PyObject *o) {
    if (o == NULL) return;
    o->refcnt--;
    if (o->refcnt == 0) {
        /* Free to make subsequent access a heap-use-after-free under ASan */
        free(o);
    }
}

static void Py_INCREF(PyObject *o) {
    if (o) o->refcnt++;
}

static int PyObject_HasAttrString(PyObject *obj, const char *name) {
    /* Accessing fields of obj after it has been freed will trigger UAF */
    if (obj == NULL) return 0;
    if (strcmp(name, "io_close") == 0) return obj->has_io_close;
    if (strcmp(name, "flush") == 0) return obj->has_flush;
    if (strcmp(name, "write") == 0) return obj->has_write;
    return 0;
}

static PyObject *PyObject_CallMethod(PyObject *obj, const char *name, const char *fmt) {
    /* Touch the object to ensure ASan sees a read from freed memory in callbacks */
    if (obj == NULL) return NULL;
    volatile int touch = obj->magic; /* read-after-free when obj is freed */
    (void)touch;
    /* Return a new dummy object to be DECREF'ed by the caller */
    PyObject *ret = (PyObject *)malloc(sizeof(PyObject));
    if (ret) {
        ret->refcnt = 1;
        ret->has_io_close = ret->has_flush = ret->has_write = 0;
        ret->magic = 0xBEEF;
    }
    return ret;
}

/* Callbacks as in python/libxml.c */
static int xmlPythonFileClose(void * context) {
    PyObject *file, *ret = NULL;

    file = (PyObject *) context;
    if (file == NULL) return(-1);
    if (PyObject_HasAttrString(file, "io_close")) {
        ret = PyObject_CallMethod(file, "io_close", "()");
    } else if (PyObject_HasAttrString(file, "flush")) {
        ret = PyObject_CallMethod(file, "flush", "()");
    }
    if (ret != NULL) {
        Py_DECREF(ret);
    }
    return(0);
}

static int xmlPythonFileWrite(void * context, const char * buffer, int len) {
    PyObject *file = (PyObject *) context;
    if (file == NULL) return -1;
    /* Touch the object. If it was freed, ASan will flag UAF. */
    volatile int touch = file->magic;
    (void)touch;
    return len;
}

/* Minimal stand-in for xmlOutputBufferCreateIO */
static xmlOutputBufferPtr xmlOutputBufferCreateIO(xmlOutputWriteCallback iowrite,
                                                  xmlOutputCloseCallback ioclose,
                                                  void *ioctx,
                                                  xmlCharEncodingHandlerPtr encoder) {
    xmlOutputBufferPtr buf = (xmlOutputBufferPtr)malloc(sizeof(xmlOutputBuffer));
    if (!buf) return NULL;
    buf->writecallback = iowrite;
    buf->closecallback = ioclose;
    buf->context = ioctx;
    buf->encoder = encoder;
    return buf;
}

/* Vulnerable constructor: passes raw PyObject* without INCREF */
static xmlOutputBufferPtr xmlOutputBufferCreatePythonFile(PyObject *file,
                                                         xmlCharEncodingHandlerPtr encoder) {
    xmlOutputBufferPtr ret;

    if (file == NULL) return NULL;

    /* No Py_INCREF(file) here -> buffer holds a non-owned raw pointer */
    ret = xmlOutputBufferCreateIO(xmlPythonFileWrite, xmlPythonFileClose, file, encoder);

    return ret;
}

int main(void) {
    /* Create a fake Python file object with a single reference */
    PyObject *file = (PyObject *)malloc(sizeof(PyObject));
    if (!file) return 1;
    file->refcnt = 1;                 /* single owning ref */
    file->has_io_close = 1;           /* make close path call into object */
    file->has_flush = 1;
    file->has_write = 1;
    file->magic = 0xDEADBEEF;

    /* Create the libxml2 output buffer. Vulnerable code stores raw pointer. */
    xmlOutputBufferPtr buf = xmlOutputBufferCreatePythonFile(file, NULL);
    if (!buf) {
        fprintf(stderr, "Failed to create output buffer\n");
        return 1;
    }

    /* Simulate Python dropping its last reference to the file object
       while the output buffer still exists. This frees the PyObject. */
    Py_DECREF(file); /* file is now freed, but buf->context still points to it */

    /* Any subsequent callback (write/close) will use the dangling pointer. */
    if (buf->writecallback) {
        const char *data = "trigger";
        /* This will read file->magic, causing heap-use-after-free under ASan */
        buf->writecallback(buf->context, data, (int)strlen(data));
    }

    if (buf->closecallback) {
        /* This will call PyObject_HasAttrString on the freed PyObject */
        buf->closecallback(buf->context);
    }

    free(buf);

    fprintf(stderr, "If compiled with ASan, a heap-use-after-free should have been reported.\n");
    return 0;
}
