#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

/*
   Minimal stand-in for the contrib/mfs/mfs_file.c globals.
   The bug is that mfs_close() indexes fds[fd] without validating fd.
*/

#define MAX_BUFFS 4

static int fds[MAX_BUFFS];
static char *buf[MAX_BUFFS];

int mfs_unmap(int fd) { return 0; }

/* Vulnerable function as in the source context */
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
        fds[fd] = -1;  /* Out-of-bounds write when fd is invalid */
        ret = 0;
    }

    return ret;
}

int main(void)
{
    /* Initialize fds to something other than -1 to take the else-branch if reachable */
    for (int i = 0; i < MAX_BUFFS; i++)
        fds[i] = 0;

    /* Craft an invalid fd that is out of bounds. */
    int badfd = MAX_BUFFS + 10;  /* >= MAX_BUFFS => out-of-bounds */

    /* This call will access fds[badfd] without any bounds check. With ASan, the
       first invalid access (the read in the 'if' condition) will already be reported.
       The core issue is the missing bounds check which also leads to the write below. */
    int r = mfs_close(badfd);

    /* Prevent optimizing away the call result */
    printf("mfs_close(%d) returned %d\n", badfd, r);

    return 0;
}
