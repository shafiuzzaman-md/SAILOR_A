#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Minimal type re-declarations matching libxml2 conventions */
typedef unsigned char xmlChar;

/* Stub xmlFree to match libxml2 API */
static void xmlFree(void *p) {
    free(p);
}

/* Stub of xmlC14NDocDumpMemory to keep control flow similar to the original */
static int xmlC14NDocDumpMemory(void *doc,
                                void *nodes,
                                int exclusive,
                                xmlChar **prefixes,
                                int with_comments,
                                xmlChar **doc_txt)
{
    (void)doc;
    (void)nodes;
    (void)exclusive;
    (void)prefixes;
    (void)with_comments;
    /* Return a small buffer to mimic success path */
    *doc_txt = (xmlChar *)malloc(1);
    if (*doc_txt) **doc_txt = 0;
    return 0; /* success */
}

/*
 * Reimplementation of the buggy cleanup sequence from python/libxml.c
 * The bug: iterates with while (*idx) over a non-NULL-terminated array.
 */
static int libxml_C14NDocDumpMemory(void *doc,
                                    void *nodes,
                                    int exclusive,
                                    xmlChar **prefixes,
                                    int with_comments)
{
    int result;
    xmlChar *doc_txt = NULL;

    result = xmlC14NDocDumpMemory(doc, nodes, exclusive, prefixes, with_comments, &doc_txt);

    /* In the real code nodes would be freed here if non-NULL. We pass NULL. */

    /* Vulnerable cleanup: assumes prefixes is NULL-terminated, but it may not be. */
    if (prefixes) {
        xmlChar **idx = prefixes;
        /* BUG: Reads past the end of the allocated prefixes array if it's not NULL-terminated. */
        while (*idx) xmlFree(*(idx++));
        xmlFree(prefixes);
    }

    if (result >= 0 && doc_txt) {
        xmlFree(doc_txt);
    }

    return result;
}

int main(void) {
    /* Craft a prefixes array that is NOT NULL-terminated, like a buggy PystringSet_Convert output. */
    /* Allocate space for exactly one pointer (no room for a NULL terminator). */
    xmlChar **prefixes = (xmlChar **)malloc(1 * sizeof(xmlChar *));
    if (!prefixes) return 1;

    /* First (and only) entry is a non-NULL xmlChar* that will be freed once. */
    prefixes[0] = (xmlChar *)malloc(8);
    if (!prefixes[0]) return 1;
    strcpy((char *)prefixes[0], "X"); /* any non-empty string */

    /* Call the function that contains the vulnerable cleanup loop. */
    /* nodes=NULL to skip unrelated frees; exclusive/with_comments are irrelevant to the bug. */
    /* This will free prefixes[0], then evaluate while (*idx) at idx==&prefixes[1], which is OOB. */
    (void)libxml_C14NDocDumpMemory(NULL, NULL, 1, prefixes, 0);

    /* If the bug didn't trigger (it should with ASan), exit normally. */
    return 0;
}
