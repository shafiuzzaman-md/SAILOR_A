#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Minimal re-declarations of libxml2 list types */
typedef struct _xmlLink xmlLink;
typedef xmlLink* xmlLinkPtr;
struct _xmlLink {
    void *data;
    xmlLinkPtr next;
    xmlLinkPtr prev;
};

typedef struct _xmlList xmlList;
typedef xmlList* xmlListPtr;
struct _xmlList {
    xmlLinkPtr sentinel;
};

/* Helpers to build and tear down lists */
static xmlListPtr xmlListCreate(void) {
    xmlListPtr l = (xmlListPtr)malloc(sizeof(xmlList));
    if (!l) return NULL;
    l->sentinel = (xmlLinkPtr)malloc(sizeof(xmlLink));
    if (!l->sentinel) {
        free(l);
        return NULL;
    }
    l->sentinel->data = NULL;
    l->sentinel->next = l->sentinel;
    l->sentinel->prev = l->sentinel;
    return l;
}

static void xmlListDelete(xmlListPtr l) {
    if (!l) return;
    if (l->sentinel) {
        xmlLinkPtr cur = l->sentinel->next;
        while (cur && cur != l->sentinel) {
            xmlLinkPtr nxt = cur->next;
            free(cur);
            cur = nxt;
        }
        free(l->sentinel);
    }
    free(l);
}

static int xmlListEmpty(xmlList *l) {
    if (l == NULL || l->sentinel == NULL) return 1;
    return l->sentinel->next == l->sentinel;
}

static int xmlListInsert(xmlList *l, void *data) {
    if (!l || !l->sentinel) return -1;
    xmlLinkPtr node = (xmlLinkPtr)malloc(sizeof(xmlLink));
    if (!node) return -1;
    node->data = data;
    /* insert before sentinel (at tail) */
    node->next = l->sentinel;
    node->prev = l->sentinel->prev;
    l->sentinel->prev->next = node;
    l->sentinel->prev = node;
    return 0;
}

static xmlListPtr xmlListDup(xmlList *l) {
    if (!l) return NULL;
    xmlListPtr dup = xmlListCreate();
    if (!dup) return NULL;
    /* shallow-copy the data pointers */
    for (xmlLinkPtr it = l->sentinel->next; it != l->sentinel; it = it->next) {
        if (xmlListInsert(dup, it->data) != 0) {
            xmlListDelete(dup);
            return NULL;
        }
    }
    return dup;
}

static void xmlListClear(xmlList *l) {
    /* A simple clear implementation that frees all nodes and resets the list */
    if (l == NULL || l->sentinel == NULL) return;
    xmlLinkPtr it = l->sentinel->next;
    while (it != l->sentinel) {
        xmlLinkPtr nxt = it->next;
        free(it);
        it = nxt;
    }
    l->sentinel->next = l->sentinel;
    l->sentinel->prev = l->sentinel;
}

/*
 * Stubbed internal helpers to simulate the bug scenario.
 * xmlListCopy(from, to) will simulate allocation failure and free the
 * destination list 'to' (use-after-free source), mimicking libxml2's
 * buggy behavior on error (lines 702-704 in xmlListCopy).
 */
static int xmlListCopy(xmlList *from, xmlList *to) {
    (void)from; /* unused in this stub */

    /* Simulate an allocation failure during copy. */
    /* Buggy cleanup: free the destination list 'to'. */
    if (to != NULL) {
        xmlListDelete(to); /* Frees 'to' including its sentinel and struct */
    }

    /* Return non-zero to indicate failure; caller (xmlListMerge/xmlListSort)
       won't notice that 'to' got freed. */
    return -1;
}

static void xmlListMerge(xmlList *l, xmlList *other) {
    /* In libxml2, this merges 'other' into 'l'. Here we just call copy. */
    (void)other;
    (void)xmlListCopy(other, l);
    /* No error handling => 'l' might have been freed by xmlListCopy. */
}

/* Vulnerable function as per the provided source snippet. */
static void xmlListSort(xmlList *l) {
    xmlListPtr lTemp;

    if (l == NULL)
        return;
    if (xmlListEmpty(l))
        return;

    lTemp = xmlListDup(l);
    if (lTemp == NULL)
        return;
    xmlListClear(l);
    /* UAF bug source: xmlListMerge may free 'l' on failure, but xmlListSort
       continues and returns normally, leaving caller with dangling pointer. */
    xmlListMerge(l, lTemp);
    xmlListDelete(lTemp);
}

/* A small walker to optionally exercise list access after sort. */
static int print_walker(void *data, void *user) {
    (void)user;
    printf("walker: %p\n", data);
    return 1; /* continue */
}

static void xmlListWalk(xmlList *l, int (*walker)(void*, void*), void *user) {
    if ((l == NULL) || (walker == NULL))
        return;
    for (xmlLinkPtr lk = l->sentinel->next; lk != l->sentinel; lk = lk->next) {
        if ((walker(lk->data, user)) == 0)
            break;
    }
}

int main(void) {
    /* Build a list with at least one element so xmlListDup succeeds
       and xmlListClear actually clears something. */
    xmlListPtr l = xmlListCreate();
    if (!l) {
        fprintf(stderr, "failed to create list\n");
        return 1;
    }

    /* Insert a couple of dummy items */
    int *x = (int*)malloc(sizeof(int));
    int *y = (int*)malloc(sizeof(int));
    if (!x || !y) return 1;
    *x = 1; *y = 2;
    xmlListInsert(l, x);
    xmlListInsert(l, y);

    /* Trigger the vulnerable path: this will call xmlListMerge(l, lTemp),
       which calls xmlListCopy and frees 'l' on (simulated) failure. */
    xmlListSort(l);

    /* Use-after-free: 'l' was freed inside xmlListSort via xmlListCopy.
       Accessing 'l' now should trigger ASan heap-use-after-free. */
    fprintf(stderr, "About to trigger UAF by accessing freed list...\n");

    /* Read from freed object */
    if (l && l->sentinel) {
        /* This read dereferences memory inside the freed 'l' allocation. */
        fprintf(stderr, "sentinel ptr: %p\n", (void*)l->sentinel);
    }

    /* Write into freed object to make the UAF unmistakable. */
    l->sentinel = NULL; /* ASan should flag this as heap-use-after-free */

    /* Optionally, try walking (will also deref freed memory). */
    xmlListWalk(l, print_walker, NULL);

    /* Cleanup dangling resources (x,y) to avoid unrelated leaks in output */
    free(x);
    free(y);

    return 0;
}
