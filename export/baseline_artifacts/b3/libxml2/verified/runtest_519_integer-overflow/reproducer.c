#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>

/*
 * Stub xmlMalloc/xmlFree that simulate a 32-bit allocator on a 64-bit host.
 * This intentionally truncates the requested size to 32 bits to emulate the
 * off_t -> size_t truncation issue described, so a huge requested size wraps
 * around to a tiny allocation.
 */
void *xmlMalloc(size_t size) {
    uint32_t s32 = (uint32_t)size; /* emulate 32-bit size_t truncation */
    if (s32 == 0) {
        /* emulate allocators that return a non-NULL minimal buffer for 0 */
        s32 = 1;
    }
    void *p = malloc(s32);
    fprintf(stderr, "xmlMalloc called with size=%zu (0x%zx), truncated to %u bytes, ptr=%p\n",
            size, size, s32, p);
    return p;
}

void xmlFree(void *ptr) {
    free(ptr);
}

/* Dummy I/O layer so we don't touch the real FS. */
static int my_open(const char *filename, int flags) {
    (void)filename; (void)flags;
    return 3; /* any non-negative fd */
}

static int my_close(int fd) {
    (void)fd;
    return 0;
}

/*
 * my_stat sets st_size to 2^32 - 1, so st_size + 1 == 2^32. When that value
 * is passed to xmlMalloc(size_t), our stub truncates to 32 bits, yielding 0,
 * and we allocate only 1 byte. The subsequent read loop will then overflow.
 */
static int my_stat(const char *filename, struct stat *st) {
    (void)filename;
    memset(st, 0, sizeof(*st));
    st->st_size = (off_t)((1ULL << 32) - 1ULL); /* 4294967295 */
    return 0;
}

/*
 * my_read writes some data into the provided buffer to simulate reading from a file.
 * We ignore the requested count and just write 1024 bytes once to trigger overflow.
 */
static ssize_t my_read(int fd, void *buf, size_t count) {
    (void)fd;
    static int done = 0;
    size_t to_write;
    if (done) return 0; /* EOF on second call */
    (void)count; /* ignore requested size; we control how much to write */
    to_write = 1024; /* write 1 KiB to overflow the tiny allocation */
    memset(buf, 'A', to_write);
    done = 1;
    return (ssize_t)to_write;
}

/* Minimal defines to avoid unknown macros */
#ifndef RD_FLAGS
#define RD_FLAGS 0
#endif

/* Vulnerable function adapted from runtest.c:loadMem */
static int loadMem(const char *filename, const char **mem, int *size) {
    int fd, res;
    struct stat info;
    char *base;
    int siz = 0;
    if (my_stat(filename, &info) < 0)
        return(-1);
    /* Vulnerable allocation: off_t (info.st_size) + 1 passed to xmlMalloc(size_t) */
    base = (char *)xmlMalloc((size_t)(info.st_size + 1));
    if (base == NULL)
        return(-1);
    fd = my_open(filename, RD_FLAGS);
    if (fd  < 0) {
        xmlFree(base);
        return(-1);
    }
    while ((res = (int)my_read(fd, &base[siz], (size_t)(info.st_size - (off_t)siz))) > 0) {
        siz += res;
    }
    my_close(fd);
#if !defined(_WIN32)
    if ((off_t)siz != info.st_size) {
        xmlFree(base);
        return(-1);
    }
#endif
    base[siz] = 0;
    *mem = base;
    *size = siz;
    return(0);
}

static int unloadMem(const char *mem) {
    xmlFree((char *)mem);
    return 0;
}

int main(void) {
    const char *mem = NULL;
    int size = 0;

    /* Trigger the bug. The overflow occurs inside the read loop above. */
    int ret = loadMem("dummy-big-file", &mem, &size);
    fprintf(stderr, "loadMem returned %d, size=%d, mem=%p\n", ret, size, (void*)mem);

    if (mem) unloadMem(mem);
    return 0;
}