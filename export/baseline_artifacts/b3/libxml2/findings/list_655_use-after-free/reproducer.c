#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Minimal re-declarations to model libxml2's list API and reproduce the bug */

typedef struct _xmlLink xmlLink;
typedef xmlLink * xmlLinkPtr;

typedef struct _xmlList xmlList;
typedef xmlList * xmlListPtr;

typedef int (*xmlListWalker)(const void *data, void *user);
typedef int (*xmlListCompare)(const void *a, const void *b);

struct _xmlLink {
    xmlLinkPtr next;
    xmlLinkPtr prev;
    void *data;
};

struct _xmlList {
    xmlLinkPtr sentinel;
    xmlListCompare linkCompare;
};

/* Global to simulate allocation failure inside xmlListInsert */
static int g_fail_in_next_insert = 0;

static xmlLinkPtr xmlLinkCreate(void *data) {
    xmlLinkPtr lk = (xmlLinkPtr)malloc(sizeof(xmlLink));
    if (!lk) return NULL;
    lk->data = data;
    lk->next = lk->prev = NULL;
    return lk;
}

xmlListPtr xmlListCreate(void *deallocator /*unused*/, xmlListCompare cmp) {
    (void)deallocator;
    xmlListPtr l = (xmlListPtr)malloc(sizeof(xmlList));
    if (!l) return NULL;
    l->sentinel = (xmlLinkPtr)malloc(sizeof(xmlLink));
    if (!l->sentinel) { free(l); return NULL; }
    l->sentinel->next = l->sentinel->prev = l->sentinel;
    l->sentinel->data = NULL;
    l->linkCompare = cmp;
    return l;
}

void xmlListClear(xmlList *l) {
    if (l == NULL || l->sentinel == NULL) return;
    xmlLinkPtr it = l->sentinel->next;
    while (it != l->sentinel) {
        xmlLinkPtr nxt = it->next;
        free(it);
        it = nxt;
    }
    l->sentinel->next = l->sentinel->prev = l->sentinel;
}

void xmlListDelete(xmlList *l) {
    if (l == NULL) return;
    xmlListClear(l);
    if (l->sentinel) free(l->sentinel);
    free(l);
}

/* Simplified insert: append before sentinel. Returns 0 on success, 1 on error. */
int xmlListInsert(xmlList *l, void *data) {
    if (l == NULL || l->sentinel == NULL) return 1;
    if (g_fail_in_next_insert > 0) {
        /* Simulate allocation failure inside xmlListInsert */
        g_fail_in_next_insert--;
        return 1; /* error */
    }
    xmlLinkPtr lk = xmlLinkCreate(data);
    if (!lk) return 1;
    /* Insert before sentinel (append) */
    xmlLinkPtr tail = l->sentinel->prev;
    lk->next = l->sentinel;
    lk->prev = tail;
    tail->next = lk;
    l->sentinel->prev = lk;
    return 0;
}

/* Forward declaration for walker-based functions */
void xmlListWalk(xmlList *l, xmlListWalker walker, void *user);
void xmlListReverseWalk(xmlList *l, xmlListWalker walker, void *user);

/* Vulnerable helpers, modeled after the source context */
int xmlListCopy(xmlList *cur, xmlList *old) {
    if (cur == NULL || old == NULL) return 1;
    xmlLinkPtr lk;
    for (lk = old->sentinel->next; lk != old->sentinel; lk = lk->next) {
        if (0 != xmlListInsert(cur, lk->data)) {
            /* On error, free the destination list and signal failure */
            xmlListDelete(cur);
            return 1;
        }
    }
    return 0;
}

void xmlListMerge(xmlList *l1, xmlList *l2) {
    /* BUG: return value from xmlListCopy is ignored. On failure, l1 is freed. */
    xmlListCopy(l1, l2);
    xmlListClear(l2);
}

void xmlListWalk(xmlList *l, xmlListWalker walker, void *user) {
    xmlLinkPtr lk;
    if ((l == NULL) || (walker == NULL))
        return;
    /* This will dereference l->sentinel even if l was freed => UAF */
    for (lk = l->sentinel->next; lk != l->sentinel; lk = lk->next) {
        if ((walker(lk->data, user)) == 0)
            break;
    }
}

void xmlListReverseWalk(xmlList *l, xmlListWalker walker, void *user) {
    xmlLinkPtr lk;
    if ((l == NULL) || (walker == NULL))
        return;
    for (lk = l->sentinel->prev; lk != l->sentinel; lk = lk->prev) {
        if ((walker(lk->data, user)) == 0)
            break;
    }
}

/* Simple walker that just keeps scanning */
static int print_walker(const void *data, void *user) {
    (void)user;
    printf("walker saw node %p\n", data);
    return 1; /* continue */
}

int main(void) {
    /* Create two lists and populate them */
    xmlListPtr l1 = xmlListCreate(NULL, NULL);
    xmlListPtr l2 = xmlListCreate(NULL, NULL);
    if (!l1 || !l2) {
        fprintf(stderr, "Failed to create lists\n");
        return 1;
    }

    /* Populate l1 and l2 normally (no failure yet) */
    if (xmlListInsert(l1, (void*)0x1111) != 0) return 2;
    if (xmlListInsert(l1, (void*)0x2222) != 0) return 2;

    if (xmlListInsert(l2, (void*)0xAAAA) != 0) return 2;
    if (xmlListInsert(l2, (void*)0xBBBB) != 0) return 2;

    /* Arm the failure so that the first insert during merge fails */
    g_fail_in_next_insert = 1;

    /* This will call xmlListCopy(l1, l2). On simulated failure, xmlListCopy
     * frees l1 (via xmlListDelete). xmlListMerge ignores the error and returns
     * with l1 dangling.
     */
    xmlListMerge(l1, l2);

    /* Use-after-free: l1 was freed inside xmlListCopy, but we still use it. */
    fprintf(stdout, "About to walk l1 (should trigger ASan UAF)\n");
    xmlListWalk(l1, print_walker, NULL);

    /* Clean up l2 (it's still valid but emptied by merge) */
    xmlListDelete(l2);

    /* l1 was already freed; avoid double free. End program. */
    return 0;
}
