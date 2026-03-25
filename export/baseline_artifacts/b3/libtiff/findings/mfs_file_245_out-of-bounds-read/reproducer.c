// Standalone reproducer for OOB read in mfs_lseek (contrib/mfs/mfs_file.c:245)
#include <stdio.h>
#include <errno.h>
#include <string.h>

// Minimal environment to compile the vulnerable logic
#define MAX_BUFFS 8

// Globals mimicking contrib/mfs/mfs_file.c
static int fds[MAX_BUFFS];
static int buf_size_arr[MAX_BUFFS];
static int buf_off_arr[MAX_BUFFS];
static int buf_mode_arr[MAX_BUFFS];
static char *buf_arr[MAX_BUFFS];

// Stub for internal helper referenced by mfs_lseek
static void extend_mem_file(int fd, long newsize) {
    // Minimal no-op/resize stub to satisfy references
    if (fd >= 0 && fd < MAX_BUFFS) {
        if (newsize > buf_size_arr[fd])
            buf_size_arr[fd] = (int)newsize;
    }
}

// Vulnerable function (adapted from contrib/mfs/mfs_file.c)
static int mfs_lseek(int fd, int offset, int whence)
{
    int ret;
    long test_off;

    // BUG: No bounds check on fd before indexing fds[fd]
    if (fds[fd] == -1) /* Not open */
    {
        ret = -1;
        errno = EBADF;
    }
    else if (offset < 0 && whence == SEEK_SET)
    {
        ret = -1;
        errno = EINVAL;
    }
    else
    {
        switch (whence)
        {
            case SEEK_SET:
                if (offset > buf_size_arr[fd])
                    extend_mem_file(fd, offset);
                buf_off_arr[fd] = offset;
                ret = offset;
                break;

            case SEEK_CUR:
                test_off = buf_off_arr[fd] + offset;

                if (test_off < 0)
                {
                    ret = -1;
                    errno = EINVAL;
                }
                else
                {
                    if (test_off > buf_size_arr[fd])
                        extend_mem_file(fd, test_off);
                    buf_off_arr[fd] = (int)test_off;
                    ret = (int)test_off;
                }
                break;

            case SEEK_END:
                test_off = buf_size_arr[fd] + offset;

                if (test_off < 0)
                {
                    ret = -1;
                    errno = EINVAL;
                }
                else
                {
                    if (test_off > buf_size_arr[fd])
                        extend_mem_file(fd, test_off);
                    buf_off_arr[fd] = (int)test_off;
                    ret = (int)test_off;
                }
                break;

            default:
                ret = -1;
                errno = EINVAL;
                break;
        }
    }

    return ret;
}

int main(void)
{
    // Initialize globals to some benign values
    memset(fds, 0, sizeof(fds));
    memset(buf_size_arr, 0, sizeof(buf_size_arr));
    memset(buf_off_arr, 0, sizeof(buf_off_arr));
    memset(buf_mode_arr, 0, sizeof(buf_mode_arr));
    memset(buf_arr, 0, sizeof(buf_arr));

    // Craft an out-of-bounds file descriptor: exactly one past the end
    int bad_fd = MAX_BUFFS; // >= MAX_BUFFS triggers OOB at fds[bad_fd]

    // This call will perform an out-of-bounds READ at fds[bad_fd]
    // due to missing bounds check on fd in mfs_lseek.
    int r = mfs_lseek(bad_fd, 0, SEEK_SET);

    // Prevent unused warnings and keep side effects
    printf("mfs_lseek returned %d, errno=%d\n", r, errno);
    return 0;
}
