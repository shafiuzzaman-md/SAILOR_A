#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Minimal stand-in for the chunk_info table used by pngunknown.c */
typedef struct {
    const char *name; /* only the name field is used by find() */
} chunk_info_t;

static chunk_info_t chunk_info[] = {
    { "IHDR" },
    { "IDAT" },
    { "IEND" },
    { "tEXt" },
    { "zTXt" },
    { "iTXt" },
};

#define NINFO ((int)(sizeof(chunk_info)/sizeof(chunk_info[0])))

/* Vulnerable function as in contrib/libtests/pngunknown.c:find */
static int find(const char *name)
{
    int i = NINFO;
    while (--i >= 0)
    {
        /* BUG: unconditionally compares 4 bytes from caller-provided name */
        if (memcmp(chunk_info[i].name, name, 4) == 0)
            break;
    }
    return i;
}

int main(void)
{
    /* Create a too-short buffer to trigger OOB read in memcmp(..., name, 4) */
    char *short_name = (char *)malloc(1); /* only 1 byte allocated */
    if (!short_name) {
        perror("malloc");
        return 1;
    }
    short_name[0] = 'I'; /* e.g., starts like "I..." but only 1 byte long */

    /* This call will cause memcmp to read 4 bytes from short_name, which is
       an out-of-bounds read for this 1-byte allocation. ASan should flag it. */
    int idx = find(short_name);

    /* Prevent optimizing away the call */
    printf("find returned %d (expected -1)\n", idx);

    free(short_name);
    return 0;
}