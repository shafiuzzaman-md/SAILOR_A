#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>

/*
 This is a standalone minimal reconstruction of contrib/mfs/mfs_file.c logic
 to trigger the heap-buffer-overflow in mfs_write when given a negative size.
*/

#define MAX_FD 4

static int   fds[MAX_FD];
static char *buf[MAX_FD];
static int   buf_size[MAX_FD];
static int   buf_off[MAX_FD];
static char  buf_mode[MAX_FD];

/* Stub of extend_mem_file as in the original code. For this reproducer we won't
   call it in the buggy path (condition is false when size is negative), but we
   provide it for completeness. */
static void extend_mem_file(int fd, int new_size)
{
    char *newbuf = (char *)realloc(buf[fd], (size_t)new_size);
    if (!newbuf) {
        perror("realloc");
        exit(1);
    }
    /* Zero-fill the newly added region for determinism. */
    if (new_size > buf_size[fd]) {
        memset(newbuf + buf_size[fd], 0xCD, (size_t)(new_size - buf_size[fd]));
    }
    buf[fd] = newbuf;
}

/* Vulnerable function re-implemented from contrib/mfs/mfs_file.c */
int mfs_write(int fd, void *clnt_buf, int size)
{
    int ret;

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

        /* BUG: size is int; negative size passes to memcpy as a huge size_t */
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
    /* Set up a single memory-backed file descriptor in write mode with a small buffer */
    int fd = 0;
    for (int i = 0; i < MAX_FD; i++) {
        fds[i] = -1;
        buf[i] = NULL;
        buf_size[i] = 0;
        buf_off[i] = 0;
        buf_mode[i] = 'r';
    }

    /* Open-like initialization */
    fds[fd] = fd;          /* mark as open */
    buf_mode[fd] = 'w';    /* write mode */
    buf_size[fd] = 16;     /* small destination buffer */
    buf_off[fd] = 0;
    buf[fd] = (char *)malloc((size_t)buf_size[fd]);
    if (!buf[fd]) {
        perror("malloc");
        return 1;
    }
    memset(buf[fd], 0xAA, (size_t)buf_size[fd]);

    /* Small source buffer; large enough to reach first OOB write at dest */
    size_t src_len = 32;
    char *src = (char *)malloc(src_len);
    if (!src) {
        perror("malloc src");
        return 1;
    }
    memset(src, 0xBB, src_len);

    /* Trigger: pass a negative size. The check (buf_off + size > buf_size)
       will be false (e.g., 0 + (-1) > 16 is false), so no extension occurs.
       memcpy receives size as a huge size_t, overrunning buf[fd]. */
    int neg_size = -1;

    fprintf(stderr, "About to call mfs_write with negative size = %d (will convert to size_t)\n", neg_size);
    /* This call should trigger AddressSanitizer heap-buffer-overflow */
    int ret = mfs_write(fd, src, neg_size);

    /* Normally unreachable if ASan aborts earlier; keep to avoid optimization */
    fprintf(stderr, "mfs_write returned %d, buf_off=%d, buf_size=%d\n", ret, buf_off[fd], buf_size[fd]);

    free(src);
    free(buf[fd]);
    return 0;
}
