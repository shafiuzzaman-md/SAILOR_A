#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

/* Minimal stubs and types to reproduce the null-pointer-dereference in
   _bfd_add_bfd_to_archive_cache when bfd_zalloc returns NULL. */

typedef long file_ptr;

typedef struct bfd bfd;

typedef struct {
    void *cache; /* archive cache (htab_t) */
} bfd_archdata_t;

typedef struct {
    void *parent_cache; /* htab_t */
    file_ptr key;
} areltdata_stub;

struct bfd {
    bfd_archdata_t *tdata;  /* Archive data */
    areltdata_stub *arelt;  /* Element data */
};

/* Archive cache entry. */
struct ar_cache {
    file_ptr ptr;
    bfd *arbfd;
};

/* Hash table stubs (libiberty-like). */
typedef void* htab_t;

static unsigned int hash_file_ptr(const void *p) {
    /* Dummy hash. */
    const struct ar_cache *ac = (const struct ar_cache *)p;
    return (unsigned int)(ac ? (uintptr_t)ac->ptr : 0U);
}

static int eq_file_ptr(const void *p1, const void *p2) {
    const struct ar_cache *a = (const struct ar_cache *)p1;
    const struct ar_cache *b = (const struct ar_cache *)p2;
    return a && b && a->ptr == b->ptr;
}

static void * _bfd_calloc_wrapper(size_t a, size_t b) {
    return calloc(a, b);
}

static htab_t htab_create_alloc(unsigned int size,
                                unsigned int (*hash_f)(const void*),
                                int (*eq_f)(const void*, const void*),
                                void (*del_f)(void*),
                                void *(*alloc_f)(size_t, size_t),
                                void (*free_f)(void*)) {
    (void)size; (void)hash_f; (void)eq_f; (void)del_f; (void)free_f;
    /* Return any non-NULL pointer to simulate successful creation. */
    void *tbl = alloc_f(1, 8);
    if (!tbl) tbl = (void*)0x1; /* Fallback non-NULL */
    return (htab_t)tbl;
}

enum insert_option { NO_INSERT = 0, INSERT = 1 };

static void **htab_find_slot(htab_t table, const void *entry, int insert) {
    (void)table; (void)insert;
    /* Return the address of a static slot. */
    static void *slot = NULL;
    slot = (void *)entry;
    return &slot;
}

/* Accessors mimicking bfd_ardata and arch_eltdata. */
static inline bfd_archdata_t *bfd_ardata(bfd *abfd) { return abfd->tdata; }
static inline areltdata_stub *arch_eltdata(bfd *abfd) { return abfd->arelt; }

/* Stub bfd_zalloc that forces allocation failure to trigger the bug. */
static void *bfd_zalloc(bfd *abfd, size_t size) {
    (void)abfd; (void)size;
    /* Simulate out-of-memory: return NULL. */
    return NULL;
}

/* Vulnerable function (reproduced with minimal dependencies). */
bool _bfd_add_bfd_to_archive_cache(bfd *arch_bfd, file_ptr filepos, bfd *new_elt) {
    struct ar_cache *cache;
    htab_t hash_table = bfd_ardata(arch_bfd)->cache;

    /* If the hash table hasn't been created, create it. */
    if (hash_table == NULL) {
        hash_table = htab_create_alloc(16, hash_file_ptr, eq_file_ptr,
                                       NULL, _bfd_calloc_wrapper, free);
        if (hash_table == NULL)
            return false;
        bfd_ardata(arch_bfd)->cache = hash_table;
    }

    /* Insert new_elt into the hash table by filepos. */
    cache = (struct ar_cache *) bfd_zalloc(arch_bfd, sizeof(struct ar_cache));
    /* BUG: cache is not checked for NULL, dereference causes crash. */
    cache->ptr = filepos;              /* Null-pointer dereference here */
    cache->arbfd = new_elt;
    *htab_find_slot(hash_table, (const void *)cache, INSERT) = cache;

    /* Provide a means of accessing this from child. */
    arch_eltdata(new_elt)->parent_cache = hash_table;
    arch_eltdata(new_elt)->key = filepos;

    return true;
}

int main(void) {
    /* Set up a minimal archive bfd and a new element bfd. */
    bfd *archive = (bfd *)calloc(1, sizeof(bfd));
    bfd *element = (bfd *)calloc(1, sizeof(bfd));
    if (!archive || !element) {
        fprintf(stderr, "Allocation failure in setup.\n");
        return 1;
    }

    archive->tdata = (bfd_archdata_t *)calloc(1, sizeof(bfd_archdata_t));
    element->arelt = (areltdata_stub *)calloc(1, sizeof(areltdata_stub));
    if (!archive->tdata || !element->arelt) {
        fprintf(stderr, "Allocation failure in setup (tdata/arelt).\n");
        return 1;
    }

    /* Ensure cache is NULL to exercise hashtable creation path. */
    archive->tdata->cache = NULL;

    /* This call will attempt to bfd_zalloc and then unconditionally deref it. */
    /* With our stubbed bfd_zalloc returning NULL, this triggers the crash. */
    (void)_bfd_add_bfd_to_archive_cache(archive, (file_ptr)0x1234, element);

    /* Should not reach here. */
    printf("Unexpectedly survived null dereference.\n");
    return 0;
}
