#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Minimal re-declaration of the data structures from contrib/mfs/mfs_file.c */
#define MAX_BUFFS 4

static int fds[MAX_BUFFS];
static char *buf[MAX_BUFFS];
static int buf_size[MAX_BUFFS];
static int buf_off[MAX_BUFFS];

/* Initialize the fake in-memory FS state (as in mem_init) */
static void mem_init(void)
{
    int i;
    for (i = 0; i < MAX_BUFFS; i++) {
        fds[i] = -1;
        buf[i] = (char *)NULL;
        buf_size[i] = 0;
        buf_off[i] = 0;
    }
}

/* Vulnerable function (copied from the provided context) */
static int extend_mem_file(int fd, int size)
{
    void *new_mem;
    int ret;

    /* BUG: no bounds check on fd before indexing buf[fd] */
    if ((new_mem = realloc(buf[fd], size)) == (void *)NULL)
        ret = -1;
    else {
        buf[fd] = (char *)new_mem;
        ret = 0;
    }

    return ret;
}

int main(void)
{
    mem_init();

    /* Seed some valid entries to resemble a realistic state */
    for (int i = 0; i < MAX_BUFFS; i++) {
        fds[i] = i; /* mark as open */
        buf[i] = (char *)malloc(8);
        if (buf[i]) memset(buf[i], 'A', 8);
        buf_size[i] = 8;
        buf_off[i] = 0;
    }

    /* Trigger: use an out-of-range fd so buf[fd] is an OOB read */
    int fd = MAX_BUFFS;       /* one past the end => OOB */
    int new_size = 16;

    /* This call will read buf[MAX_BUFFS] (out-of-bounds) and pass it to realloc,
       which ASan will flag as an out-of-bounds read on a global array. */
    int ret = extend_mem_file(fd, new_size);

    /* Likely not reached if ASan aborts on error, but keeps compiler from
       optimizing away the call. */
    printf("extend_mem_file returned %d\n", ret);

    return 0;
}
