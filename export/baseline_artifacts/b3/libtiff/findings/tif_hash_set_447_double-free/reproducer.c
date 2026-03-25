#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

/* Try to include the internal hash set API from libtiff if available. */
#if __has_include("libtiff/tif_hash_set.h")
#include "libtiff/tif_hash_set.h"
#elif __has_include("tif_hash_set.h")
#include "tif_hash_set.h"
#else
/* Fallback: forward declare the minimal API we need, matching libtiff's tif_hash_set.c */
typedef struct _TIFFHashSet TIFFHashSet;
typedef unsigned long (*TIFFHashSetHashFunc)(const void *);
typedef int (*TIFFHashSetEqualFunc)(const void *, const void *);
typedef void (*TIFFHashSetFreeEltFunc)(void *);

/* Expected signatures based on tif_hash_set.c */
TIFFHashSet *TIFFHashSetNew(TIFFHashSetHashFunc fnHashFunc,
                            TIFFHashSetEqualFunc fnEqualFunc,
                            TIFFHashSetFreeEltFunc fnFreeEltFunc,
                            int bRehash);
void TIFFHashSetDestroy(TIFFHashSet *set);
bool TIFFHashSetInsert(TIFFHashSet *set, void *elt);
#endif

static unsigned long ptr_hash(const void *p)
{
    /* Simple, stable pointer hash: use the address value */
    return (unsigned long)(uintptr_t)p;
}

static int ptr_equal(const void *a, const void *b)
{
    return a == b;
}

static void free_elt(void *p)
{
    /* Element free function used by the hash set */
    free(p);
}

int main(void)
{
    /* Create a hash set with pointer-hash, pointer-equality, and free() as the element free function. */
    TIFFHashSet *set = TIFFHashSetNew(ptr_hash, ptr_equal, free_elt, 0);
    if (!set)
    {
        fprintf(stderr, "Failed to create TIFFHashSet\n");
        return 1;
    }

    /* Allocate an element and insert it. */
    void *elt = malloc(16);
    if (!elt)
    {
        fprintf(stderr, "malloc failed\n");
        TIFFHashSetDestroy(set);
        return 1;
    }

    if (!TIFFHashSetInsert(set, elt))
    {
        fprintf(stderr, "First insert failed\n");
        TIFFHashSetDestroy(set);
        return 1;
    }

    /* Insert the exact same pointer again. This hits the duplicate-element path:
       - TIFFHashSetFindPtr finds the existing entry and returns a pointer to it
       - TIFFHashSetInsert calls fnFreeEltFunc(*pElt) -> free(elt)
       - Then it unconditionally assigns *pElt = elt (same freed pointer)
       The set now holds a dangling (freed) pointer. */
    if (!TIFFHashSetInsert(set, elt))
    {
        fprintf(stderr, "Second insert failed (but the bug path may still have run)\n");
    }

    /* Destroying the set will call fnFreeEltFunc on stored elements again, which
       attempts to free the same pointer a second time -> double free. ASan should report it. */
    TIFFHashSetDestroy(set);

    /* If the program reaches here without ASan aborting, explicitly exit. */
    return 0;
}
