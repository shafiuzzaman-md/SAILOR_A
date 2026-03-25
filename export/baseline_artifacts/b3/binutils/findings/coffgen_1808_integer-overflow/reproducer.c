#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>
#include <inttypes.h>

/* Self-contained stubs and types to mimic the relevant parts of BFD. */

typedef uint32_t bfd_size_type; /* Force 32-bit to reproduce the overflow. */

typedef struct bfd {
    unsigned char *filebuf;
    size_t filelen;
    size_t cursor;
    /* Fields referenced by macros below. */
    bfd_size_type symesz;
    bfd_size_type raw_syment_count;
    bfd_size_type sym_filepos;
    char *coff_strings;
    bfd_size_type coff_strings_len;
} bfd;

#define STRING_SIZE_SIZE 4

/* Macros mimicking the ones used by the vulnerable function. */
#define bfd_coff_symesz(abfd)            ((abfd)->symesz)
#define obj_raw_syment_count(abfd)       ((abfd)->raw_syment_count)
#define obj_sym_filepos(abfd)            ((abfd)->sym_filepos)
#define obj_coff_strings(abfd)           ((abfd)->coff_strings)
#define obj_coff_strings_len(abfd)       ((abfd)->coff_strings_len)

/* Minimal error handling stubs. */
enum bfd_error {
    bfd_error_no_error = 0,
    bfd_error_file_truncated,
    bfd_error_bad_value
};
static enum bfd_error last_bfd_error = bfd_error_no_error;
static void bfd_set_error(enum bfd_error e) { last_bfd_error = e; }
static enum bfd_error bfd_get_error(void) { return last_bfd_error; }

/* Simple error handler stub matching the varargs call site. */
static void _bfd_error_handler(const char *fmt, ...) {
    va_list ap; va_start(ap, fmt); vfprintf(stderr, fmt, ap); va_end(ap); fputc('\n', stderr);
}

/* Endianness helper used by the code (assume LE for simplicity). */
static inline uint32_t H_GET_32(bfd *abfd, const unsigned char *p) {
    (void)abfd; /* unused in this stub */
    return ((uint32_t)p[0]) | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

/* Minimal file ops on our in-memory buffer. */
static int bfd_seek(bfd *abfd, bfd_size_type pos, int whence) {
    (void)whence; /* only SEEK_SET used */
    abfd->cursor = (size_t)pos;
    if (abfd->cursor > abfd->filelen) abfd->cursor = abfd->filelen;
    return 0;
}

static bfd_size_type bfd_read(void *dst, bfd_size_type size, bfd *abfd) {
    size_t remaining = (abfd->cursor < abfd->filelen) ? (abfd->filelen - abfd->cursor) : 0;
    size_t to_read = (size <= remaining) ? (size_t)size : remaining;
    if (to_read > 0) memcpy(dst, abfd->filebuf + abfd->cursor, to_read);
    abfd->cursor += to_read;
    return (bfd_size_type)to_read;
}

static bfd_size_type bfd_get_file_size(bfd *abfd) {
    (void)abfd;
    /* Return 0 to bypass the upper-bound check (filesize != 0 && strsize > filesize). */
    return 0;
}

static void *bfd_malloc(bfd_size_type n) {
    /* On 32-bit bfd_size_type, overflow before this call can turn n into 0. */
    return malloc((size_t)n);
}

/* Overflow helper. */
static bool _bfd_mul_overflow(bfd_size_type a, bfd_size_type b, bfd_size_type *res) {
    uint64_t prod = (uint64_t)a * (uint64_t)b;
    *res = (bfd_size_type)prod;
    return prod > 0xFFFFFFFFu;
}

/* Vulnerable function ported with stubs above. */
static char * _bfd_coff_read_string_table(bfd *abfd) {
    bfd_size_type symesz, size, pos;
    unsigned char extstrsize[STRING_SIZE_SIZE];
    bfd_size_type strsize;
    bfd_size_type filesize;
    char *strings;

    symesz = bfd_coff_symesz(abfd);
    pos = obj_sym_filepos(abfd);
    if (_bfd_mul_overflow(obj_raw_syment_count(abfd), symesz, &size)
        || pos + size < pos)
    {
        bfd_set_error(bfd_error_file_truncated);
        return NULL;
    }

    if (bfd_seek(abfd, pos + size, SEEK_SET) != 0)
        return NULL;

    if (bfd_read(extstrsize, sizeof extstrsize, abfd) != sizeof extstrsize)
    {
        if (bfd_get_error() != bfd_error_file_truncated)
            return NULL;
        /* There is no string table. */
        strsize = STRING_SIZE_SIZE;
    }
    else
    {
        strsize = H_GET_32(abfd, extstrsize);
    }

    filesize = bfd_get_file_size(abfd);
    if (strsize < STRING_SIZE_SIZE || (filesize != 0 && strsize > filesize))
    {
        _bfd_error_handler("%s: bad string table size %" PRIu64, "abfd", (uint64_t)strsize);
        bfd_set_error(bfd_error_bad_value);
        return NULL;
    }

    /* Vulnerable allocation: on 32-bit bfd_size_type, strsize == 0xFFFFFFFF wraps here to 0. */
    strings = (char *)bfd_malloc(strsize + 1);
    if (strings == NULL)
        return NULL;

    /* Heap buffer overflow when strsize + 1 wrapped to 0 above. */
    memset(strings, 0, STRING_SIZE_SIZE);

    if (bfd_read(strings + STRING_SIZE_SIZE, strsize - STRING_SIZE_SIZE, abfd)
        != strsize - STRING_SIZE_SIZE)
    {
        free(strings);
        return NULL;
    }

    obj_coff_strings(abfd) = strings;
    obj_coff_strings_len(abfd) = strsize;
    /* Terminate the string table, just in case. */
    strings[strsize] = 0;
    return strings;
}

int main(void) {
    /* Prepare an in-memory "file" whose first 4 bytes are 0xFFFFFFFF -> strsize. */
    unsigned char buf[4] = { 0xFF, 0xFF, 0xFF, 0xFF };

    bfd abfd;
    memset(&abfd, 0, sizeof(abfd));
    abfd.filebuf = buf;
    abfd.filelen = sizeof(buf);
    abfd.cursor = 0;
    abfd.symesz = 0;              /* so size = 0 */
    abfd.raw_syment_count = 0;    /* so size = 0 */
    abfd.sym_filepos = 0;         /* start at 0 */

    /* This call should trigger the integer overflow leading to a heap buffer overflow
       in the memset(strings, 0, 4) when bfd_malloc(0) returned. */
    char *s = _bfd_coff_read_string_table(&abfd);

    /* If we reached here without ASan aborting, free any allocation and report. */
    if (s) free(s);
    puts("Done");
    return 0;
}
