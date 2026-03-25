// Standalone C reproducer for the out-of-bounds read in store_memory_free
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Minimal typedefs to mirror libpng test harness types */
typedef unsigned char png_byte;
typedef size_t png_alloc_size_t;

/* Forward-declare a dummy png_struct to match the signature */
typedef struct png_struct_def { int dummy; } png_struct;

/* Forward declaration of png_store to satisfy store_pool */
typedef struct png_store_s png_store;

/* Minimal store_memory representation used by the test harness */
typedef struct store_memory_s {
    struct store_memory_s *next;
    struct store_pool_s *pool;
    png_alloc_size_t size;   /* number of bytes requested by the caller */
    png_byte mark[16];       /* start marker */
} store_memory;

/* Minimal store_pool used by the test harness */
typedef struct store_pool_s {
    png_store *store;
    png_alloc_size_t max;
    png_alloc_size_t current;
    store_memory *list;
    png_byte mark[16];       /* both start and end markers use this pattern */
} store_pool;

/* Minimal png_store and logging stubs */
struct png_store_s {
    const char *test;
    int nerrors;
    const char *wname;
    void *current;
};

static void store_log(png_store *ps, const png_struct *pp, const char *msg, int is_error)
{
    (void)ps; (void)pp; (void)is_error;
    fprintf(stderr, "store_log: %s\n", msg);
}

/* This mirrors the helper in pngvalid.c */
static void store_pool_error(png_store *ps, const png_struct *pp, const char *msg)
{
    (void)pp; /* We pass pp==NULL in this reproducer to avoid png_error */
    store_log(ps, pp, msg, 1 /* error */);
}

/* Vulnerable function as in contrib/libtests/pngvalid.c */
static void store_memory_free(const png_struct *pp, store_pool *pool, store_memory *memory)
{
    if (memory->pool != pool)
        store_pool_error(pool->store, (const png_struct*)pp, "memory corrupted (pool)");

    else if (memcmp(memory->mark, pool->mark, sizeof memory->mark) != 0)
        store_pool_error(pool->store, (const png_struct*)pp, "memory corrupted (start)");

    /* It should be safe to read the size field now. */
    else
    {
        png_alloc_size_t cb = memory->size;

        if (cb > pool->max)
            store_pool_error(pool->store, (const png_struct*)pp, "memory corrupted (size)");

        /* BUG: If the original allocation was too small, the pointer below
         * advances past the end of the allocated block and memcmp reads OOB.
         */
        else if (memcmp((png_byte *)(memory+1)+cb, pool->mark, sizeof pool->mark) != 0)
            store_pool_error(pool->store, (const png_struct*)pp, "memory corrupted (end)");

        /* Finally give the library a chance to find problems too: */
        else
        {
            pool->current -= cb;
            free(memory);
        }
    }
}

int main(void)
{
    /* Set up a pool with a known marker pattern */
    store_pool pool;
    memset(&pool, 0, sizeof(pool));
    for (size_t i = 0; i < sizeof pool.mark; ++i) pool.mark[i] = (png_byte)(0xA5 ^ (int)i);
    pool.max = 1024; /* Allow sizes up to 1024 so size check passes */

    /* Allocate ONLY the header (store_memory), as if an earlier integer overflow
     * in the allocator produced a too-small block that lacks the trailing marker.
     */
    store_memory *memory = (store_memory*)malloc(sizeof(store_memory));
    if (!memory) {
        fprintf(stderr, "malloc failed\n");
        return 1;
    }

    memset(memory, 0, sizeof(*memory));
    memory->pool = &pool;                 /* Correct pool pointer so first check passes */
    memcpy(memory->mark, pool.mark, sizeof memory->mark); /* Start marker matches */
    memory->size = 1;                     /* Pretend user requested 1 byte */

    /* This call will perform:
     *   memcmp((png_byte*)(memory+1) + cb, pool->mark, sizeof pool->mark)
     * memory+1 is just past the header we allocated; adding cb=1 advances one
     * more byte beyond the allocated region, so memcmp reads OOB.
     */
    store_memory_free(NULL /* pp==NULL to avoid png_error path */, &pool, memory);

    /* If we get here without ASan aborting, free the header to avoid a leak. */
    free(memory);
    return 0;
}
