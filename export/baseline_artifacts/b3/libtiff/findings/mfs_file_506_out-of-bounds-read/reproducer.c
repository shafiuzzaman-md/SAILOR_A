#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

/* Match the vulnerable code's expectations: fds is sized by MAX_BUFFS */
#define MAX_BUFFS 8

static int fds[MAX_BUFFS];

static void mem_init(void)
{
    for (int i = 0; i < MAX_BUFFS; i++)
        fds[i] = -1; /* mark all as not open */
}

/* Vulnerable function (mirrors contrib/mfs/mfs_file.c:mfs_close) */
int mfs_close(int fd)
{
    int ret;

    if (fds[fd] == -1) /* Not open */
    {
        ret = -1;
        errno = EBADF;
    }
    else
    {
        fds[fd] = -1;
        ret = 0;
    }

    return (ret);
}

int main(void)
{
    mem_init();

    /* Craft an out-of-bounds index: one past the end of fds[] */
    int bad_fd = MAX_BUFFS;  /* >= MAX_BUFFS triggers OOB read on fds[fd] */

    /* Triggers the out-of-bounds read at: if (fds[fd] == -1) */
    int ret = mfs_close(bad_fd);

    /* Keep side effects visible */
    printf("mfs_close(%d) -> %d, errno=%d\n", bad_fd, ret, errno);
    return 0;
}
