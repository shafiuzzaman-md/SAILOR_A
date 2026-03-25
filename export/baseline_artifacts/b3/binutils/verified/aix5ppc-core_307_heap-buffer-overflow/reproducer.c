#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

/* Minimal bfd stand-ins */
typedef struct bfd {
    const unsigned char *data;
    size_t size;
    size_t pos;
    const char *filename;
} bfd;

static int bfd_seek(bfd *abfd, long offset, int whence) {
    if (whence != SEEK_SET) return -1;
    if (offset < 0 || (size_t)offset > abfd->size) return -1;
    abfd->pos = (size_t)offset;
    return 0;
}

static size_t bfd_read(void *buf, size_t size, bfd *abfd) {
    if (abfd->pos >= abfd->size) return 0;
    size_t remain = abfd->size - abfd->pos;
    if (remain < size) size = remain;
    memcpy(buf, abfd->data + abfd->pos, size); /* This write will underflow on bug */
    abfd->pos += size;
    return size;
}

static char *bfd_malloc(size_t n) { return (char *)malloc(n); }
static char *bfd_realloc(void *p, size_t n) { return (char *)realloc(p, n); }
static const char *bfd_get_filename(bfd *abfd) { return abfd && abfd->filename ? abfd->filename : ""; }

/* Minimal core header struct used by the vulnerable function */
struct core_dumpxx {
    uint64_t c_loader; /* Offset in the core to the path string */
};

/* Vulnerable function copied and minimally adapted to our stubs. */
static bool xcoff64_core_file_matches_executable_p(bfd *core_bfd, bfd *exec_bfd) {
    struct core_dumpxx core;
    char *path, *s;
    size_t alloc;
    const char *str1, *str2;
    bool return_value = false;

    /* Get the header. */
    if (bfd_seek(core_bfd, 0, SEEK_SET) != 0)
        return return_value;

    if (sizeof(struct core_dumpxx) != bfd_read(&core, sizeof(struct core_dumpxx), core_bfd))
        return return_value;

    if (bfd_seek(core_bfd, (long)core.c_loader, SEEK_SET) != 0)
        return return_value;

    alloc = 100;
    path = bfd_malloc(alloc);
    if (path == NULL)
        return return_value;

    s = path;

    while (1) {
        if (bfd_read(s, 1, core_bfd) != 1)
            goto xcoff64_core_file_matches_executable_p_end_1;

        if (*s == '\0')
            break;
        ++s;
        if (s == path + alloc) {
            char *n;
            alloc *= 2;
            n = bfd_realloc(path, alloc);
            if (n == NULL)
                goto xcoff64_core_file_matches_executable_p_end_1;

            /* BUG: wrong pointer adjustment (should be: s = n + (s - path)) */
            s = n + (path - s);
            path = n;
        }
    }

    str1 = strrchr(path, '/');
    str2 = strrchr(bfd_get_filename(exec_bfd), '/');

    /* Step over character '/'. */
    str1 = str1 != NULL ? str1 + 1 : path;
    str2 = str2 != NULL ? str2 + 1 : bfd_get_filename(exec_bfd);

    if (strcmp(str1, str2) == 0)
        return_value = true;

xcoff64_core_file_matches_executable_p_end_1:
    free(path);
    return return_value;
}

int main(void) {
    /* Build a fake core "file": [header][path bytes...] */
    const size_t header_size = sizeof(struct core_dumpxx);

    /* Path of length 101 (no NUL yet) to force one realloc at 100, then one more byte write. */
    const size_t path_len_no_nul = 101; /* > 100 to trigger the reallocation */
    const size_t total_size = header_size + path_len_no_nul + 1; /* +1 for terminating NUL */

    unsigned char *blob = (unsigned char *)malloc(total_size);
    if (!blob) return 1;

    struct core_dumpxx hdr;
    hdr.c_loader = (uint64_t)header_size; /* path starts right after header */
    memcpy(blob, &hdr, header_size);

    /* Fill 101 non-zero bytes so the loop crosses the 100-byte boundary, then a NUL to stop */
    memset(blob + header_size, 'A', path_len_no_nul);
    blob[header_size + path_len_no_nul] = '\0';

    bfd core = { blob, total_size, 0, "fake-core" };
    bfd exec = { NULL, 0, 0, "/bin/fake-exec" };

    /* This call should trigger an ASan heap-buffer-underflow in our bfd_read memcpy */
    (void)xcoff64_core_file_matches_executable_p(&core, &exec);

    free(blob);
    return 0;
}