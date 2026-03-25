#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>

/* Minimal in-memory file state mirroring contrib/mfs/mfs_file.c */
#define MAXFDS 4
static unsigned char *buf[MAXFDS];
static int buf_off[MAXFDS];
static int buf_size[MAXFDS];
static char buf_mode[MAXFDS];
static int fds[MAXFDS];

/* Stub of extend_mem_file: grow buffer when asked to, no-op otherwise. */
static void extend_mem_file(int fd, int new_size)
{
    /* In the real code this ensures capacity >= new_size. For our repro,
       we only grow if larger is requested; negative or smaller requests are ignored. */
    if (new_size > buf_size[fd]) {
        unsigned char *newp = (unsigned char *)realloc(buf[fd], (size_t)new_size);
        if (!newp) {
            perror("realloc");
            exit(1);
        }
        buf[fd] = newp;
        /* Note: mfs_write also sets buf_size[fd], so we don't here. */
    }
}

/* Vulnerable function (append mode path) */
int mfs_write(int fd, void *clnt_buf, int size)
{
    int ret;

    if (fds[fd] == -1 || buf_mode[fd] == 'r')
    {
        ret = -1;
        errno = EBADF;
    }
    else if (buf_mode[fd] == 'w')
    {
        if (buf_off[fd] + size > buf_size[fd])
        {
            extend_mem_file(fd, buf_off[fd] + size);
            buf_size[fd] = (buf_off[fd] + size);
        }

        memcpy((buf[fd] + buf_off[fd]), clnt_buf, (size_t)size);
        buf_off[fd] = buf_off[fd] + size;

        ret = size;
    }
    else
    {
        /* Append */
        if (buf_off[fd] != buf_size[fd])
            buf_off[fd] = buf_size[fd];

        extend_mem_file(fd, buf_off[fd] + size);
        buf_size[fd] += size;  /* Decreases when size is negative */

        /* BUG: size is an int; a negative value gets converted to huge size_t */
        memcpy((buf[fd] + buf_off[fd]), clnt_buf, (size_t)size);
        buf_off[fd] = buf_off[fd] + size;

        ret = size;
    }

    return (ret);
}

int main(void)
{
    /* Initialize state */
    for (int i = 0; i < MAXFDS; i++) {
        fds[i] = -1;
        buf[i] = NULL;
        buf_off[i] = 0;
        buf_size[i] = 0;
        buf_mode[i] = '\0';
    }

    int fd = 0;
    fds[fd] = 3;        /* Mark as open */
    buf_mode[fd] = 'a'; /* Any value other than 'r' or 'w' takes the append path */
    buf_size[fd] = 16;  /* Small heap buffer */
    buf_off[fd] = 0;
    buf[fd] = (unsigned char *)malloc((size_t)buf_size[fd]);
    if (!buf[fd]) {
        perror("malloc");
        return 1;
    }
    memset(buf[fd], 'A', (size_t)buf_size[fd]);

    /* Tiny source buffer; destination overflow is what we want ASan to report */
    char *src = (char *)malloc(1);
    if (!src) {
        perror("malloc");
        return 1;
    }
    src[0] = 'B';

    /* Trigger: negative size leads to huge size_t in memcpy in append mode */
    int neg_size = -1;  /* Will become SIZE_MAX when cast to size_t */

    /* This call should immediately trigger ASan heap-buffer-overflow in memcpy */
    (void)mfs_write(fd, src, neg_size);

    /* Clean up (unlikely reached due to ASan abort) */
    free(src);
    free(buf[fd]);
    return 0;
}
