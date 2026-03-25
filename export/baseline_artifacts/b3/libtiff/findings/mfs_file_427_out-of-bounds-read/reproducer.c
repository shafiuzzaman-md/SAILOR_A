#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

/*
 This standalone reproducer emulates the vulnerable data layout from
 contrib/mfs/mfs_file.c and calls mfs_size() with an out-of-range fd,
 causing an out-of-bounds read on fds[fd].
*/

#define MAX_FDS 4

/* Emulate the global arrays used by contrib/mfs/mfs_file.c */
char *buf[MAX_FDS];
int fds[MAX_FDS];
int buf_size[MAX_FDS];
int buf_off[MAX_FDS];

/* Vulnerable function copied in spirit from contrib/mfs/mfs_file.c */
int mfs_size(int fd)
{
    int ret;

    /* No bounds check on fd before indexing fds[fd] */
    if (fds[fd] == -1) /* Not open */
    {
        ret = -1;
        errno = EBADF;
    }
    else
        ret = buf_size[fd];

    return ret;
}

int main(void)
{
    /* Initialize arrays to some valid state for in-range descriptors */
    for (int i = 0; i < MAX_FDS; i++) {
        fds[i] = 0;          /* mark as open */
        buf_size[i] = 1234;  /* arbitrary size */
        buf_off[i] = 0;
        buf[i] = NULL;
    }

    /* Craft an invalid fd that is out of bounds for the arrays */
    int bad_fd = MAX_FDS + 12; /* clearly out-of-bounds index */

    /* This call triggers an out-of-bounds read on fds[bad_fd] */
    int sz = mfs_size(bad_fd);

    /* Prevent unused warnings and keep the compiler from optimizing away */
    printf("mfs_size(%d) = %d\n", bad_fd, sz);

    return 0;
}
