#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

/* Minimal recreation of the vulnerable globals and function from contrib/mfs/mfs_file.c */

#define MAX_BUFFS 8

/* Global state arrays (no bounds checks on index in mfs_read) */
static int fds[MAX_BUFFS];
static int buf_size[MAX_BUFFS];
static int buf_off[MAX_BUFFS];
static char buf_mode[MAX_BUFFS];
static unsigned char *buf[MAX_BUFFS];

/* Vulnerable function: missing bounds check on fd before indexing */
int mfs_read(int fd, void *clnt_buf, int size)
{
    int ret;

    if (fds[fd] == -1 || buf_mode[fd] != 'r')
    {
        /* File is either not open, or not opened for read */
        ret = -1;
        errno = EBADF;
    }
    else if (buf_off[fd] + size > buf_size[fd])
    {
        ret = 0; /* EOF */
    }
    else
    {
        memcpy(clnt_buf, (void *)(buf[fd] + buf_off[fd]), size);
        buf_off[fd] = buf_off[fd] + size;
        ret = size;
    }

    return (ret);
}

int main(void)
{
    /* Initialize a small, valid in-bounds state for demonstration */
    for (int i = 0; i < MAX_BUFFS; i++) {
        fds[i] = i;              /* mark as open */
        buf_mode[i] = 'r';       /* readable */
        buf_size[i] = 16;
        buf_off[i] = 0;
        buf[i] = (unsigned char *)malloc(buf_size[i]);
        if (buf[i]) memset(buf[i], 'A' + i, buf_size[i]);
    }

    char out[4] = {0};

    /* Trigger the bug: pass a negative fd, which results in indexing fds[-1] and buf_mode[-1] */
    int ret = mfs_read(-1, out, 1);

    /* Print something to keep the call/result observable; ASan should report before this returns normally */
    printf("mfs_read(-1, ...) returned %d, errno=%d\n", ret, errno);

    for (int i = 0; i < MAX_BUFFS; i++) {
        free(buf[i]);
    }

    /* Also attempt an out-of-upper-bounds index if execution continues (usually ASan halts earlier) */
    (void)mfs_read(MAX_BUFFS, out, 1);

    return 0;
}
