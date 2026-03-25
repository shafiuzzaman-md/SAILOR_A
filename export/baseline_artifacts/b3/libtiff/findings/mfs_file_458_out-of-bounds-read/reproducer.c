#include <stdio.h>
#include <errno.h>
#include <stddef.h>

/* Minimal re-declaration of the globals used by contrib/mfs/mfs_file.c */
#define MAX_FDS 4
static int fds[MAX_FDS];
static char *buf[MAX_FDS];
static size_t buf_size[MAX_FDS];

/* Vulnerable function copied in spirit from contrib/mfs/mfs_file.c */
int mfs_map(int fd, char **addr, size_t *len)
{
    int ret;

    /* BUG: No bounds check on fd before indexing fds[fd] */
    if (fds[fd] == -1) /* Not open */
    {
        ret = -1;
        errno = EBADF;
    }
    else
    {
        *addr = buf[fd];
        *len = buf_size[fd];
        ret = 0;
    }

    return ret;
}

static void mfs_init(void)
{
    for (int i = 0; i < MAX_FDS; i++) {
        fds[i] = -1;      /* mark all as not open */
        buf[i] = NULL;
        buf_size[i] = 0;
    }
}

int main(void)
{
    mfs_init();

    /* Choose an fd that is out of bounds for the small fds[] array */
    int bad_fd = MAX_FDS + 1; /* e.g., 5 when MAX_FDS is 4 */

    char *addr = NULL;
    size_t len = 0;

    /* This call will perform an out-of-bounds read on fds[bad_fd] */
    int ret = mfs_map(bad_fd, &addr, &len);

    /* Prevent optimizing away the call / variables */
    printf("mfs_map returned %d (errno=%d). addr=%p len=%zu\n", ret, errno, (void*)addr, len);

    return 0;
}
