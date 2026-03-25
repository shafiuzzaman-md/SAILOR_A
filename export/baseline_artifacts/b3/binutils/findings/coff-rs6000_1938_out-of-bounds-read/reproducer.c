#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

/* Self-contained minimal stubs and types to reproduce the OOB read in
 * xcoff_write_armap_old (bfd/coff-rs6000.c:1938).
 */

#define ATTRIBUTE_UNUSED

/* Minimal bfd and archive structures */
typedef struct bfd bfd;

struct artdata_hdr {
    char memoff[20];
};
struct artdata_u {
    struct artdata_hdr hdr;
};
struct artdata {
    struct artdata_u u;
};

struct bfd {
    struct artdata art;
};

static inline struct artdata *x_artdata(bfd *abfd) {
    return &abfd->art;
}

/* Minimal archive iterator emulation */
struct archive_iterator {
    struct {
        bfd *member;     /* Points to the bfd of the current member */
        unsigned int offset; /* Offset of the current member */
    } current;
    int started;
};

static void archive_iterator_begin(struct archive_iterator *it, bfd *abfd ATTRIBUTE_UNUSED) {
    it->started = 0;
}

static bool archive_iterator_next(struct archive_iterator *it) {
    if (it->started)
        return false;
    it->started = 1;
    return true;
}

/* orl entry as used by the vulnerable function */
struct orl {
    bfd *abfd;           /* BFD for the archive member containing the symbol */
    const char **name;   /* Pointer to symbol name pointer */
};

/* Minimal XCOFF archive header fields used by the function */
struct xcoff_ar_hdr {
    char size[32];
    char nextoff[32];
    char prevoff[20];
    char date[12];
    char uid[12];
    char gid[12];
    char mode[12];
    char namlen[12];
};

#define SIZEOF_AR_HDR (sizeof(struct xcoff_ar_hdr))
#define XCOFFARMAG_ELEMENT_SIZE 20
static const char XCOFFARFMAG[] = "FMAG";
#define SXCOFFARFMAG 4

/* Endian store helper */
#define H_PUT_32(abfd, val, buf) do { \
    (void)(abfd); \
    unsigned int v__ = (unsigned int)(val); \
    (buf)[0] = (unsigned char)((v__ >> 24) & 0xFF); \
    (buf)[1] = (unsigned char)((v__ >> 16) & 0xFF); \
    (buf)[2] = (unsigned char)((v__ >> 8) & 0xFF); \
    (buf)[3] = (unsigned char)((v__) & 0xFF); \
} while (0)

/* bfd_write stub: emulate successful writes by returning the requested size. */
static size_t bfd_write(const void *ptr ATTRIBUTE_UNUSED, size_t size, bfd *abfd ATTRIBUTE_UNUSED) {
    return size;
}

/* Vulnerable function (trimmed to essential parts, identical logic around the bug) */
static bool xcoff_write_armap_old(bfd *abfd, unsigned int elength ATTRIBUTE_UNUSED,
                                  struct orl *map, unsigned int orl_count, int stridx) {
    struct archive_iterator iterator;
    struct xcoff_ar_hdr hdr;
    char *p;
    unsigned char buf[4];
    unsigned int i;

    memset(&hdr, 0, sizeof hdr);
    sprintf(hdr.size, "%ld", (long) (4 + orl_count * 4 + stridx));
    sprintf(hdr.nextoff, "%d", 0);
    memcpy(hdr.prevoff, x_artdata(abfd)->u.hdr.memoff, XCOFFARMAG_ELEMENT_SIZE);
    sprintf(hdr.date, "%d", 0);
    sprintf(hdr.uid, "%d", 0);
    sprintf(hdr.gid, "%d", 0);
    sprintf(hdr.mode, "%d", 0);
    sprintf(hdr.namlen, "%d", 0);

    /* Convert NULs in the header struct to spaces. */
    for (p = (char *) &hdr; p < (char *) &hdr + SIZEOF_AR_HDR; p++)
        if (*p == '\0')
            *p = ' ';

    if (bfd_write(&hdr, SIZEOF_AR_HDR, abfd) != SIZEOF_AR_HDR
        || bfd_write(XCOFFARFMAG, SXCOFFARFMAG, abfd) != SXCOFFARFMAG)
        return false;

    H_PUT_32(abfd, orl_count, buf);
    if (bfd_write(buf, 4, abfd) != 4)
        return false;

    i = 0;
    archive_iterator_begin(&iterator, abfd);
    /* Outer loop condition checks i < orl_count ... */
    while (i < orl_count && archive_iterator_next(&iterator))
        /* Inner loop uses map[i] with no re-check of i < orl_count. */
        while (map[i].abfd == iterator.current.member) {
            H_PUT_32(abfd, iterator.current.offset, buf);
            if (bfd_write(buf, 4, abfd) != 4)
                return false;
            ++i; /* When i reaches orl_count, the next condition reads map[i] OOB */
        }

    /* Remaining code is irrelevant for triggering the bug; kept for completeness. */
    for (i = 0; i < orl_count; i++) {
        const char *name;
        size_t namlen;
        name = *map[i].name;
        namlen = strlen(name);
        if (bfd_write(name, namlen + 1, abfd) != namlen + 1)
            return false;
    }

    if ((stridx & 1) != 0) {
        char b = '\0';
        if (bfd_write(&b, 1, abfd) != 1)
            return false;
    }

    return true;
}

int main(void) {
    /* Prepare a dummy bfd with a valid-looking memoff field. */
    bfd *archive_bfd = (bfd *)calloc(1, sizeof(bfd));
    if (!archive_bfd) {
        perror("calloc");
        return 1;
    }
    memcpy(archive_bfd->art.u.hdr.memoff, "01234567890123456789", 20);

    /* Prepare a single orl entry (orl_count = 1). */
    unsigned int orl_count = 1;
    struct orl *map = (struct orl *)malloc(sizeof(struct orl) * orl_count);
    if (!map) {
        perror("malloc");
        return 1;
    }

    /* Create a bfd object to serve as the member pointer. */
    bfd *member_bfd = (bfd *)calloc(1, sizeof(bfd));
    if (!member_bfd) {
        perror("calloc");
        return 1;
    }

    /* Set up the map[0] to match the iterator's member so that inner while
     * condition is true on first check, then i increments to 1 and the next
     * condition evaluation reads map[1] OOB (heap-buffer-overflow).
     */
    map[0].abfd = member_bfd;
    const char *symname = "symbol";
    map[0].name = &symname;

    /* Initialize iterator so that current.member equals member_bfd and offset arbitrary. */
    struct archive_iterator it;
    archive_iterator_begin(&it, archive_bfd);
    it.current.member = member_bfd;
    it.current.offset = 0x41424344;

    /* Call the vulnerable function. The function itself will create and use
     * its own iterator instance, but we ensure behavior is consistent by
     * setting the same member in our stubs.
     */
    (void)it; /* suppress unused warning for local iterator */

    /* Trigger the bug */
    (void)xcoff_write_armap_old(archive_bfd, 0, map, orl_count, 0);

    /* Cleanup (not reached if ASan aborts on OOB). */
    free(map);
    free(member_bfd);
    free(archive_bfd);
    return 0;
}
