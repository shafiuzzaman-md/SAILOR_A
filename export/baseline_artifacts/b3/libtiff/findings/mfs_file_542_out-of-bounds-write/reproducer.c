#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

/*
 This reproducer mirrors the vulnerable logic from contrib/mfs/mfs_file.c:
 - global arrays representing per-fd state
 - mem_init() initializes them
 - extend_mem_file(fd, size) reallocates buf[fd] and writes back to buf[fd]
 The bug: no bounds check on fd, so buf[fd] may be an out-of-bounds write.
*/

#define MAX_BUFFS 8

static int   fds[MAX_BUFFS];
static char *buf[MAX_BUFFS];
static int   buf_size[MAX_BUFFS];
static int   buf_off[MAX_BUFFS];

static void mem_init(void)
{
    int i;
    for (i = 0; i < MAX_BUFFS; i++)
    {
        fds[i] = -1;
        buf[i] = (char *)NULL;
        buf_size[i] = 0;
        buf_off[i] = 0;
    }
}

/*
 To ensure ASan reports the out-of-bounds WRITE (not the prior out-of-bounds READ
 of buf[fd] used as realloc's first argument), we read buf[fd] in a helper with
 ASan disabled. This mirrors the same out-of-bounds access but prevents ASan
 from aborting before the write occurs.
*/
__attribute__((no_sanitize_address))
static inline void *unsafe_get_buf_slot(int fd)
{
    return buf[fd];
}

/* Vulnerable function (mirrors the source context) */
static int extend_mem_file(int fd, int size)
{
    void *new_mem;
    int ret;

    /* Out-of-bounds READ may happen here via unsafe_get_buf_slot(fd) */
    void *old = unsafe_get_buf_slot(fd);

    if ((new_mem = realloc(old, size)) == (void *)NULL)
        ret = -1;
    else
    {
        /* Out-of-bounds WRITE here when fd is invalid */
        buf[fd] = (char *)new_mem;
        ret = 0;
    }

    return ret;
}

int main(void)
{
    mem_init();

    /*
     * Choose an invalid fd just past the end of buf[].
     * Layout is: fds[], buf[], buf_size[], buf_off[].
     * Accessing buf[MAX_BUFFS] will actually land in buf_size[0].
     * mem_init() set buf_size[0] = 0, so unsafe_get_buf_slot() returns NULL.
     * realloc(NULL, size) succeeds like malloc(size), and then the assignment
     * buf[MAX_BUFFS] = new_mem performs an out-of-bounds WRITE that ASan reports.
     */
    int bad_fd = MAX_BUFFS;  /* OOB index */
    int size = 32;

    int ret = extend_mem_file(bad_fd, size);

    /* Prevent optimizing away (though -O0 is used) */
    printf("extend_mem_file(bad_fd=%d, size=%d) returned %d\n", bad_fd, size, ret);

    return 0;
}
