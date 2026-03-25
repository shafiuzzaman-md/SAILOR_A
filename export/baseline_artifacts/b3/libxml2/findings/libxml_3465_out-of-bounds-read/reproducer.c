#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Minimal type re-declarations to mimic libxml2/python binding context */
typedef unsigned char xmlChar;
typedef struct _xmlDoc { int dummy; } xmlDoc, *xmlDocPtr;
typedef void* xmlNodePtr;
typedef struct _xmlNodeSet { int nodeNr; xmlNodePtr *nodeTab; } xmlNodeSet, *xmlNodeSetPtr;
typedef struct _xmlOutputBuffer { FILE *file; } xmlOutputBuffer, *xmlOutputBufferPtr;
typedef void PyObject;

/* Stubs for libxml2 memory functions */
void xmlFree(void *ptr) { free(ptr); }

/* Stubs for output buffer handling */
xmlOutputBufferPtr xmlOutputBufferCreateFile(FILE *file, void *encoder) {
    (void)encoder;
    xmlOutputBufferPtr buf = (xmlOutputBufferPtr)malloc(sizeof(xmlOutputBuffer));
    if (!buf) abort();
    buf->file = file;
    return buf;
}
int xmlOutputBufferClose(xmlOutputBufferPtr buf) {
    if (buf) free(buf);
    return 0;
}

/* Stub for canonicalization call; we don't need real work here */
int xmlC14NDocSaveTo(xmlDocPtr doc,
                     xmlNodeSetPtr nodes,
                     int exclusive,
                     xmlChar **prefixes,
                     int with_comments,
                     xmlOutputBufferPtr buf) {
    (void)doc; (void)nodes; (void)exclusive; (void)prefixes; (void)with_comments; (void)buf;
    return 0; /* success */
}

/* Python binding stubs */
void PyErr_SetString(PyObject *exc, const char *msg) { (void)exc; (void)msg; }
FILE *PyFile_Get(PyObject *py_file) { (void)py_file; return tmpfile(); }
void PyFile_Release(FILE *f) { if (f) fclose(f); }
PyObject *PyLong_FromLong(long v) { (void)v; return (PyObject*)0x1; }

/* Converters used by the vulnerable function */
int PyxmlNodeSet_Convert(PyObject *obj, xmlNodeSetPtr *nodes) {
    (void)obj;
    *nodes = NULL; /* keep it NULL so cleanup doesn't touch nodes */
    return 0; /* success */
}

/* BUG TRIGGER: returns a non-NULL-terminated array of xmlChar* */
int PystringSet_Convert(PyObject *obj, xmlChar ***out_prefixes) {
    (void)obj;
    size_t n = 1; /* one entry, no terminator */
    xmlChar **arr = (xmlChar**)malloc(n * sizeof(xmlChar*));
    if (!arr) abort();
    /* allocate a dummy string entry */
    arr[0] = (xmlChar*)malloc(4);
    if (!arr[0]) abort();
    strcpy((char*)arr[0], "x");
    *out_prefixes = arr;
    return 0; /* success */
}

/* Vulnerable function body adapted to be callable directly */
PyObject *libxml_C14NDocSaveTo(void) {
    FILE *output;
    xmlOutputBufferPtr buf;
    xmlDocPtr doc = NULL;
    xmlNodeSetPtr nodes = NULL;
    xmlChar **prefixes = NULL;
    int result;
    int with_comments = 0;
    int exclusive = 1; /* ensure prefixes conversion happens */
    int len;

    /* Set up a file/output buffer */
    output = PyFile_Get(NULL);
    if (output == NULL) {
        PyErr_SetString(NULL, "bad file.");
        return NULL;
    }
    buf = xmlOutputBufferCreateFile(output, NULL);

    /* Nodes conversion stub (keeps nodes == NULL) */
    result = PyxmlNodeSet_Convert(NULL, &nodes);
    if (result < 0) {
        xmlOutputBufferClose(buf);
        return NULL;
    }

    /* Exclusive C14N prefixes conversion: this crafts a non-NULL-terminated array */
    if (exclusive) {
        result = PystringSet_Convert(NULL, &prefixes);
        if (result < 0) {
            if (nodes) {
                xmlFree(nodes->nodeTab);
                xmlFree(nodes);
            }
            xmlOutputBufferClose(buf);
            return NULL;
        }
    }

    /* Call into C14N (stubbed) */
    result = xmlC14NDocSaveTo(doc, nodes, exclusive, prefixes, with_comments, buf);

    /* Cleanup mirrors the vulnerable code: this is where the OOB read happens */
    if (nodes) {
        xmlFree(nodes->nodeTab);
        xmlFree(nodes);
    }
    if (prefixes) {
        xmlChar ** idx = prefixes;
        while (*idx) xmlFree(*(idx++)); /* OOB read here: array is not NULL-terminated */
        xmlFree(prefixes);
    }

    PyFile_Release(output);
    len = xmlOutputBufferClose(buf);

    if (result < 0) {
        PyErr_SetString(NULL, "libxml2 xmlC14NDocSaveTo failure.");
        return NULL;
    } else {
        return PyLong_FromLong((long) len);
    }
}

int main(void) {
    /* Simply call the vulnerable function; ASan will report the OOB */
    (void)libxml_C14NDocSaveTo();
    return 0;
}
