#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/*
 * This reproducer simulates the Python wrapper bug in python/libxml.c where
 * libxml_xmlOutputBufferClose frees the underlying xmlOutputBufferPtr but the
 * wrapper object isn't updated to clear its internal pointer. A subsequent call
 * to xmlOutputBufferFlush then uses the freed pointer, causing a UAF.
 *
 * We provide minimal stub implementations of the involved libxml2 types and
 * functions to make the UAF visible under ASan without relying on libxml2.
 */

typedef struct _xmlOutputBuffer {
    uint32_t magic;
    size_t size;
    unsigned char *data;
} xmlOutputBuffer, *xmlOutputBufferPtr;

/* Minimal fake constructor for an output buffer */
static xmlOutputBufferPtr create_fake_xmlOutputBuffer(void) {
    xmlOutputBufferPtr out = (xmlOutputBufferPtr)malloc(sizeof(*out));
    if (!out) {
        perror("malloc");
        exit(1);
    }
    out->magic = 0xC0FFEEu;
    out->size = 64;
    out->data = (unsigned char *)malloc(out->size);
    if (!out->data) {
        perror("malloc");
        exit(1);
    }
    memset(out->data, 'X', out->size);
    return out;
}

/* Stub of libxml2 API: close frees the xmlOutputBuffer struct */
int xmlOutputBufferClose(xmlOutputBufferPtr out) {
    if (out == NULL)
        return -1;
    /* Intentionally free only the outer struct (as libxml2 would at least free the struct).
       Keeping out->data allocated ensures we don't double-free in this stub. */
    free(out);
    return 0;
}

/* Stub of libxml2 API: flush touches the buffer fields, which will UAF if 'out' was freed */
int xmlOutputBufferFlush(xmlOutputBufferPtr out) {
    if (out == NULL)
        return -1;
    /* Access members of 'out' to trigger use-after-free when 'out' was freed. */
    volatile uint32_t m = out->magic;            /* read from freed memory */
    if (out->data && out->size > 0) {
        /* Also write via the freed object to make the UAF obvious. */
        size_t idx = (size_t)(m % out->size);
        out->data[idx] ^= 0xAA;                  /* write guided by freed metadata */
    }
    return (int)m;
}

/* Minimal stand-in for the Python wrapper object around xmlOutputBufferPtr */
typedef struct {
    xmlOutputBufferPtr obj;   /* The wrapped C pointer */
    void *context;            /* Unused here */
    void *closecallback;      /* Unused here */
} PyoutputBuffer_Object;

/* Getter used by the wrapper to fetch the underlying C pointer */
static void *PyoutputBuffer_Get(void *pyobj) {
    return ((PyoutputBuffer_Object *)pyobj)->obj;
}

/* "Python" binding that mirrors the vulnerable logic from python/libxml.c */
static int py_libxml_xmlOutputBufferClose(PyoutputBuffer_Object *pyobj_out) {
    xmlOutputBufferPtr out = (xmlOutputBufferPtr)PyoutputBuffer_Get(pyobj_out);
    if (out == NULL)
        return 0; /* harmless if already cleared */
    /* Vulnerability: close frees 'out' but the wrapper object is NOT updated. */
    int rc = xmlOutputBufferClose(out);
    /* BUG: missing pyobj_out->obj = NULL; */
    return rc;
}

/* Another binding that uses the wrapper to get the pointer again and flush */
static int py_libxml_xmlOutputBufferFlush(PyoutputBuffer_Object *pyobj_out) {
    xmlOutputBufferPtr out = (xmlOutputBufferPtr)PyoutputBuffer_Get(pyobj_out);
    return xmlOutputBufferFlush(out);
}

int main(void) {
    /* Set up a fake Python wrapper around an xmlOutputBufferPtr */
    PyoutputBuffer_Object pybuf;
    memset(&pybuf, 0, sizeof(pybuf));
    pybuf.obj = create_fake_xmlOutputBuffer();

    /* Close frees the underlying C object but the wrapper still holds the pointer. */
    (void)py_libxml_xmlOutputBufferClose(&pybuf);

    /* This call retrieves and uses the freed pointer -> heap-use-after-free. */
    int r = py_libxml_xmlOutputBufferFlush(&pybuf);

    /* Prevent compiler from optimizing away 'r'. */
    printf("flush rc: %d\n", r);

    return 0;
}
