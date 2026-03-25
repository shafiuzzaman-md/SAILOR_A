#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Minimal stub types and flags to mirror BFD structures used by the vulnerable code. */
typedef unsigned char bfd_byte;

typedef struct asection {
    const char *name;
    unsigned int flags;
    size_t size;
    bfd_byte *contents;
} asection;

typedef struct bfd {
    asection *sec; /* single-section stub */
} bfd;

struct bfd_link_info { int dummy; };

#define SEC_HAS_CONTENTS 0x1
#define SEC_CODE         0x2
#define SEC_READONLY     0x4

/* Vulnerable helper: mimics libiberty startswith implementation. */
static int startswith(const char *s, const char *prefix) {
    /* This intentionally compares strlen(prefix) bytes unconditionally,
       which will read past the end of s if there are fewer bytes left. */
    return strncmp(s, prefix, strlen(prefix)) == 0;
}

/* Helper from the source context (unchanged). */
static char *get_name(char *ptr, char **dst) {
    while (*ptr == ' ')
        ptr++;
    *dst = ptr;
    while (*ptr && *ptr != ' ')
        ptr++;
    *ptr = 0;
    return ptr + 1;
}

/* Stubs for the subset of BFD API used by the function. */
static asection *bfd_get_section_by_name(bfd *abfd, const char *name) {
    if (abfd && abfd->sec && abfd->sec->name && strcmp(abfd->sec->name, name) == 0)
        return abfd->sec;
    return NULL;
}

static int bfd_malloc_and_get_section(bfd *abfd, asection *sec, bfd_byte **out_copy) {
    (void)abfd;
    if (!sec || !out_copy) return 0;
    *out_copy = (bfd_byte *)malloc(sec->size);
    if (!*out_copy) return 0;
    memcpy(*out_copy, sec->contents, sec->size);
    return 1;
}

/* The vulnerable function, adapted minimally to call startswith on s without
   ensuring enough bytes remain in [s, e). */
static int process_embedded_commands(bfd *output_bfd,
                                    struct bfd_link_info *info /* ATTRIBUTE_UNUSED */,
                                    bfd *abfd) {
    (void)output_bfd;
    (void)info;

    asection *sec = bfd_get_section_by_name(abfd, ".drectve");
    char *s;
    char *e;
    bfd_byte *copy;

    if (sec == NULL || (sec->flags & SEC_HAS_CONTENTS) == 0)
        return 1;

    if (!bfd_malloc_and_get_section(abfd, sec, &copy)) {
        free(copy);
        return 0;
    }
    e = (char *)copy + sec->size;

    for (s = (char *)copy; s < e; ) {
        if (s[0] != '-') {
            s++;
            continue;
        }
        /* BUG: This may read 5 bytes from s even if fewer remain before e. */
        if (startswith(s, "-attr")) {
            char *name;
            char *attribs;
            asection *asec;
            int loop = 1;
            int had_write = 0;
            int had_exec = 0;

            s += 5;
            s = get_name(s, &name);
            s = get_name(s, &attribs);

            while (loop) {
                switch (*attribs++) {
                case 'W': had_write = 1; break;
                case 'R': break;
                case 'S': break;
                case 'X': had_exec = 1; break;
                default: loop = 0; break;
                }
            }
            asec = bfd_get_section_by_name(abfd, name);
            if (asec) {
                if (had_exec) asec->flags |= SEC_CODE;
                if (!had_write) asec->flags |= SEC_READONLY;
            }
        } else if (startswith(s, "-heap")) {
            /* Not reached in this reproducer; included to mirror structure. */
            s++;
        } else {
            s++;
        }
    }

    /* Intentional leak of copy, matching original code path that doesn't free here. */
    return 1;
}

int main(void) {
    /* Create a .drectve section whose last (and only) byte is '-' so that
       s points to '-' with fewer than 5 bytes remaining, triggering OOB read
       in startswith(s, "-attr"). */
    static bfd_byte drectve_contents[1] = {'-'}; /* Only 1 byte available */

    asection drectve = {
        .name = ".drectve",
        .flags = SEC_HAS_CONTENTS,
        .size = sizeof(drectve_contents),
        .contents = drectve_contents
    };

    bfd abfd = { .sec = &drectve };

    /* Call the vulnerable function. ASan should report an OOB read. */
    process_embedded_commands(NULL, NULL, &abfd);

    return 0;
}
