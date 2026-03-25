#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Vulnerable helper from bfd/cofflink.c */
static char *
get_name(char *ptr, char **dst)
{
    while (*ptr == ' ')
        ptr++;
    *dst = ptr;
    while (*ptr && *ptr != ' ')
        ptr++;
    *ptr = 0;
    return ptr + 1;
}

int main(void)
{
    /* Allocate a small buffer that mimics a slice of a .drectve section.
       Critically, the last byte is neither NUL nor space, so parsing a
       token that starts at the last byte forces a one-past-end read. */
    size_t sz = 8;
    char *copy = (char *)malloc(sz);
    if (!copy) {
        perror("malloc");
        return 1;
    }

    /* Fill with non-space, non-NUL characters. */
    memset(copy, 'A', sz);

    /* Point to the final byte inside the allocated buffer. */
    char *ptr = copy + sz - 1;  /* in-bounds */

    /* Destination pointer for the parsed name. */
    char *name = NULL;

    /* This call will:
       - Read *ptr at the last valid byte (it's 'A'), not a space or NUL
       - Increment ptr to one past the end of the allocation
       - Evaluate *ptr in the while condition again, dereferencing
         beyond the buffer, which ASan flags as an out-of-bounds read. */
    (void)get_name(ptr, &name);

    /* We should never reach here under ASan. */
    printf("Parsed name starts at: %p (first char: %d)\n", (void *)name, (int)(unsigned char)*name);

    free(copy);
    return 0;
}
