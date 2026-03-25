#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Custom allocator that always fails to simulate malloc returning NULL. */
static void *my_malloc(size_t sz) {
    (void)sz;
    return NULL; /* Force allocation failure */
}

/* Redirect malloc used in the vulnerable code to our failing allocator. */
#define malloc my_malloc

/* Reimplementation of the vulnerable function from contrib/libtests/makepng.c */
static char *
strstash_list(const char * const *text)
{
    size_t foo = 0;
    char *result, *bar;
    const char * const *line = text;

    while (*line != NULL)
        foo += strlen(*line++);

    result = bar = malloc(foo+1); /* returns NULL in this reproducer */

    line = text;
    while (*line != NULL)
    {
        foo = strlen(*line);
        /* bar is NULL; foo > 0 for our input; this memcpy dereferences NULL. */
        memcpy(bar, *line++, foo);
        bar += foo;
    }

    *bar = 0;
    return result;
}

int main(void)
{
    /* Non-empty strings ensure memcpy length > 0, which will crash when dest is NULL. */
    const char *text[] = { "A", "BC", NULL };

    /* Expect a crash (NULL-pointer dereference) inside strstash_list. */
    char *r = strstash_list(text);

    /* Prevent unused variable warning in case behavior changes. */
    if (r)
        printf("unexpected: %s\n", r);

    return 0;
}