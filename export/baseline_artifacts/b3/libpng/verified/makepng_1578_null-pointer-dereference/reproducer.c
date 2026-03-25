#include <string.h>
#include <stddef.h>

/* Deterministic failing allocator used only inside strstash via a macro. */
static void *fail_malloc(size_t size) {
    (void)size;
    return NULL; /* Simulate malloc failure deterministically */
}

/* Vulnerable function reproduced from contrib/libtests/makepng.c
 * The malloc result is not checked before strcpy is used. */
static char *
strstash(const char *foo)
{
    if (foo != NULL)
    {
        /* Force malloc() to fail only for this allocation without
         * affecting the rest of the program or libc. */
#define malloc fail_malloc
        char *bar = malloc(strlen(foo)+1);
#undef malloc
        /* bar is NULL here; this strcpy dereferences a NULL pointer */
        return strcpy(bar, foo);
    }

    return NULL;
}

int main(void)
{
    /* Non-NULL input to take the malloc/strcpy path */
    const char *input = "trigger";

    /* This call will crash inside strcpy due to NULL destination (bar) */
    (void)strstash(input);

    return 0; /* Unreached */
}