#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

/* Minimal stubs to emulate the parts of BFD used by the vulnerable code. */
typedef unsigned char bfd_byte;

#define SEC_HAS_CONTENTS 0x1

typedef struct asection {
    const char *name;
    unsigned int flags;
    size_t size;
    bfd_byte *contents;
} asection;

typedef struct bfd {
    asection *drectve;
} bfd;

struct bfd_link_info { int dummy; };

/* Simple startswith implementation matching the usage in cofflink.c */
static int startswith(const char *s, const char *prefix) {
    size_t lp = strlen(prefix);
    return strncmp(s, prefix, lp) == 0;
}

/* Stub: return the .drectve section by name. */
static asection *bfd_get_section_by_name(bfd *abfd, const char *name) {
    if (abfd && abfd->drectve && abfd->drectve->name && strcmp(name, abfd->drectve->name) == 0)
        return abfd->drectve;
    return NULL;
}

/* Stub: allocate and copy section contents into a fresh heap buffer. */
static int bfd_malloc_and_get_section(bfd *abfd, asection *sec, bfd_byte **out) {
    (void)abfd;
    if (!sec || !out) return 0;
    bfd_byte *buf = (bfd_byte*)malloc(sec->size);
    if (!buf) return 0;
    memcpy(buf, sec->contents, sec->size);
    *out = buf;
    return 1;
}

/* Vulnerable helper, verbatim structure from cofflink.c */
static char *get_name(char *ptr, char **dst) {
    while (*ptr == ' ')
        ptr++;
    *dst = ptr;
    while (*ptr && *ptr != ' ')
        ptr++;
    /* Vulnerability: Unconditional write without bounds check. */
    *ptr = 0;  /* Potential 1-byte heap OOB write if ptr == e (one past). */
    return ptr + 1;
}

/* Minimal subset of process_embedded_commands to reach get_name on data at end of buffer. */
static int process_embedded_commands(bfd *output_bfd, struct bfd_link_info *info, bfd *abfd) {
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
        if (startswith(s, "-attr")) {
            char *name = NULL;
            /* Skip the token "-attr" (5 chars) and any following spaces are handled by get_name. */
            s += 5;
            /* This call will scan until a space or NUL. Our buffer ends right after the name
               with no space or NUL, so get_name will advance ptr past the end and then write one
               byte out of bounds when it does *ptr = 0. */
            s = get_name(s, &name);
            (void)name; /* Silence unused warning. */
            break; /* We already triggered the bug. */
        } else {
            s++;
        }
    }

    free(copy);
    return 1;
}

int main(void) {
    /* Construct a .drectve section payload that ends immediately after the name token:
       "-attr foo" with no terminating space or NUL in the section data. */
    static const char payload[] = "-attr foo"; /* NUL-terminated in C array, but we store without NUL. */
    size_t sec_size = sizeof(payload) - 1;     /* Exclude the trailing NUL from the section content. */

    /* Prepare the fake section and BFD objects. */
    asection *sec = (asection *)calloc(1, sizeof(*sec));
    bfd *abfd = (bfd *)calloc(1, sizeof(*abfd));

    if (!sec || !abfd) {
        fprintf(stderr, "Allocation failed\n");
        return 1;
    }

    sec->name = ".drectve";
    sec->flags = SEC_HAS_CONTENTS;
    sec->size = sec_size;
    sec->contents = (bfd_byte *)malloc(sec_size);
    if (!sec->contents) {
        fprintf(stderr, "Allocation failed\n");
        return 1;
    }
    memcpy(sec->contents, payload, sec_size); /* Copy without the NUL terminator. */

    abfd->drectve = sec;

    /* Any non-NULL output_bfd/info stubs are fine; they are unused in this minimal path. */
    bfd dummy_out = {0};
    struct bfd_link_info dummy_info = {0};

    /* Trigger the vulnerable parsing path. With ASan, this should report a heap-buffer-overflow
       at the write in get_name (one byte past the allocated copy buffer). */
    (void)process_embedded_commands(&dummy_out, &dummy_info, abfd);

    /* Cleanup (won't be reached if ASan aborts, but included for completeness). */
    free(sec->contents);
    free(sec);
    free(abfd);

    return 0;
}
