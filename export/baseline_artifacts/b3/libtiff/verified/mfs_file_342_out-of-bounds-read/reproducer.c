#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>

/* Minimal re-declaration of the memory-file subsystem state used by mfs_read */
#define MAXFDS 4
static int fds[MAXFDS];            /* -1 means unopened, any other value means open */
static char buf_mode[MAXFDS];       /* 'r' for read, 'w' for write, etc. */
static int buf_off[MAXFDS];         /* current offset */
static int buf_size[MAXFDS];        /* total size */
static unsigned char *buf[MAXFDS];  /* backing buffers */

/* Vulnerable function as in contrib/mfs/mfs_file.c */
static int mfs_read(int fd, void *clnt_buf, int size)
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
        /* EOF */
        ret = 0;
    }
    else
    {
        /* BUG: negative size not rejected; implicit signed->unsigned converts to huge size_t */
        memcpy(clnt_buf, (void *)(buf[fd] + buf_off[fd]), size);
        buf_off[fd] = buf_off[fd] + size;
        ret = size;
    }

    return ret;
}

int main(void)
{
    /* Initialize all fds to unopened */
    for (int i = 0; i < MAXFDS; i++) {
        fds[i] = -1;
        buf_mode[i] = '\0';
        buf_off[i] = 0;
        buf_size[i] = 0;
        buf[i] = NULL;
    }

    /* Set up a single "file" open for reading with a small backing buffer */
    int fd = 0;
    fds[fd] = 3;               /* any non -1 value to mark as open */
    buf_mode[fd] = 'r';        /* opened for read */
    buf_off[fd] = 0;
    buf_size[fd] = 16;         /* small source to make OOB read obvious */
    buf[fd] = (unsigned char *)malloc(buf_size[fd]);
    if (!buf[fd]) {
        perror("malloc");
        return 1;
    }
    /* Fill source with a known pattern */
    for (int i = 0; i < buf_size[fd]; i++) buf[fd][i] = (unsigned char)(0xA0 + i);

    /* Destination buffer is also small; the bug will attempt to copy a huge size */
    unsigned char *dst = (unsigned char *)malloc(16);
    if (!dst) {
        perror("malloc dst");
        return 1;
    }
    memset(dst, 0, 16);

    /* Trigger: pass a negative size. This bypasses EOF check and becomes a massive size_t in memcpy */
    int negative_size = -1;
    fprintf(stderr, "Calling mfs_read with size=%d (will convert to huge size_t in memcpy)\n", negative_size);

    /* This call should trigger AddressSanitizer with an out-of-bounds read from buf[fd] */
    int ret = mfs_read(fd, dst, negative_size);

    /* Normally unreachable if ASan aborts, but keep some output for completeness */
    fprintf(stderr, "mfs_read returned %d, errno=%d\n", ret, errno);

    /* Cleanup (not reached if ASan aborts) */
    free(dst);
    free(buf[fd]);
    return 0;
}