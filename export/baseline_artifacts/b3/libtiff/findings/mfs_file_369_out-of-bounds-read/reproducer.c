#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

/* Minimal stand-in for contrib/mfs/mfs_file.c globals */
#define MAX_BUFFS 4

static int   fds[MAX_BUFFS];
static char  buf_mode[MAX_BUFFS];
static int   buf_off[MAX_BUFFS];
static int   buf_size[MAX_BUFFS];
static unsigned char *buf[MAX_BUFFS];

/* Stub for internal function referenced by mfs_write */
static void extend_mem_file(int fd, int newsize) {
    (void)fd; (void)newsize; /* no-op for reproducer */
}

/* Vulnerable function (mirrors the relevant logic) */
static int mfs_write(int fd, void *clnt_buf, int size)
{
    int ret;

    /* BUG: no bounds check on fd before indexing fds[fd] and buf_mode[fd] */
    if (fds[fd] == -1 || buf_mode[fd] == 'r')
    {
        /* Either the file is not open or it is opened for reading only */
        ret = -1;
        errno = EBADF;
    }
    else if (buf_mode[fd] == 'w')
    {
        /* Write */
        if (buf_off[fd] + size > buf_size[fd])
        {
            extend_mem_file(fd, buf_off[fd] + size);
            buf_size[fd] = (buf_off[fd] + size);
        }

        memcpy((buf[fd] + buf_off[fd]), clnt_buf, size);
        buf_off[fd] = buf_off[fd] + size;

        ret = size;
    }
    else
    {
        /* Append */
        if (buf_off[fd] != buf_size[fd])
            buf_off[fd] = buf_size[fd];

        extend_mem_file(fd, buf_off[fd] + size);
        buf_size[fd] += size;

        memcpy((buf[fd] + buf_off[fd]), clnt_buf, size);
        buf_off[fd] = buf_off[fd] + size;

        ret = size;
    }

    return (ret);
}

int main(void)
{
    /* Initialize globals to benign values so normal in-bounds use would be fine */
    for (int i = 0; i < MAX_BUFFS; i++) {
        fds[i] = 0;            /* pretend they are open */
        buf_mode[i] = 'w';     /* writable mode */
        buf_off[i] = 0;
        buf_size[i] = 16;
        buf[i] = (unsigned char*)malloc(buf_size[i]);
        if (!buf[i]) return 1;
        memset(buf[i], 0xAB, buf_size[i]);
    }

    char data = 'X';

    /* Trigger: pass a negative fd to force out-of-bounds read in fds[fd] */
    /* This will read fds[-1] (and potentially buf_mode[-1]) before any bounds checks */
    int fd = -1;  /* Out-of-bounds index */
    (void)mfs_write(fd, &data, 1);

    /* Cleanup (unlikely reached if ASan aborts on OOB) */
    for (int i = 0; i < MAX_BUFFS; i++) {
        free(buf[i]);
    }

    return 0;
}
