#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>

/* Minimal stubs and types to mimic BFD pieces used by bfd_arm_update_notes. */

typedef long file_ptr;

/* Section flags. */
#define SEC_HAS_CONTENTS 0x1

/* Minimal section representation. */
typedef struct asection {
    const char *name;
    unsigned int flags;
    size_t size;
    unsigned char *contents;
} asection;

/* Minimal bfd representation holding one section and a machine value. */
typedef struct bfd_ {
    unsigned long mach;
    asection *sect; /* the only section we care about */
} bfd;

/* Machine enums used by the switch in bfd_arm_update_notes. */
enum {
    bfd_mach_arm_unknown = 0,
    bfd_mach_arm_2,
    bfd_mach_arm_2a,
    bfd_mach_arm_3,
    bfd_mach_arm_3M,
    bfd_mach_arm_4,
    bfd_mach_arm_4T,
    bfd_mach_arm_5,
    bfd_mach_arm_5T,
    bfd_mach_arm_5TE,
    bfd_mach_arm_XScale,
    bfd_mach_arm_iWMMXt,
    bfd_mach_arm_iWMMXt2
};

/* NOTE: These are just stubs to satisfy the code path. */
static asection *bfd_get_section_by_name(bfd *abfd, const char *name) {
    (void)name; /* In this stub, always return our single section */
    return abfd->sect;
}

static unsigned long bfd_get_mach(bfd *abfd) {
    return abfd->mach;
}

static bool bfd_malloc_and_get_section(bfd *abfd, asection *sec, void **buffer) {
    (void)abfd;
    if (!sec || !buffer) return false;
    *buffer = malloc(sec->size);
    if (!*buffer) return false;
    memcpy(*buffer, sec->contents, sec->size);
    return true;
}

static bool bfd_set_section_contents(bfd *abfd, asection *sec, const void *data,
                                     file_ptr offset, size_t count) {
    (void)abfd; (void)sec; (void)data; (void)offset; (void)count;
    return true; /* Not relevant for triggering the bug */
}

static void _bfd_error_handler(const char *fmt, ...) {
    va_list ap; va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fputc('\n', stderr);
    va_end(ap);
}

/* A minimal arm_Note layout to satisfy offsetof usage in the code path. */
typedef struct {
    uint32_t namesz;
    uint32_t descsz;
    uint32_t type;
    char name[1];
} arm_Note;

/* The note name key used in calls; value does not matter for this stub. */
#define NOTE_ARCH_STRING "arch"

/* Vulnerable helper: It returns a pointer into the note's descriptor string
   without ensuring NUL termination within buffer_size. For reproducer, we just
   set it to point inside the provided buffer. */
static bool arm_check_note(bfd *abfd, unsigned char *buffer, size_t buffer_size,
                           const char *note_key, const char **arch_string_out) {
    (void)abfd; (void)note_key;
    /* Point to the beginning of the buffer, which we will craft to contain
       exactly "armv2" with no trailing NUL. strcmp later will read one byte
       past the buffer while checking for the terminator. */
    if (buffer_size == 0) return false;
    *arch_string_out = (const char *)buffer; /* No guarantee of NUL within buffer_size */
    return true;
}

/* Reimplementation of the vulnerable function with minimal dependencies. */
static bool bfd_arm_update_notes(bfd *abfd) {
    static const char *note_section = ".note.arm.arch"; /* arbitrary name */
    asection *arm_arch_section;
    const char *arch_string = NULL;
    const char *expected = NULL;
    size_t buffer_size;
    unsigned char *buffer = NULL;

    arm_arch_section = bfd_get_section_by_name(abfd, note_section);

    if (arm_arch_section == NULL || (arm_arch_section->flags & SEC_HAS_CONTENTS) == 0)
        return true;

    buffer_size = arm_arch_section->size;
    if (buffer_size == 0)
        return false;

    if (!bfd_malloc_and_get_section(abfd, arm_arch_section, (void **)&buffer))
        goto FAIL;

    /* Parse the note. */
    if (!arm_check_note(abfd, buffer, buffer_size, NOTE_ARCH_STRING, &arch_string))
        goto FAIL;

    /* Check the architecture in the note against the architecture of the bfd. */
    switch (bfd_get_mach(abfd)) {
    default:
    case bfd_mach_arm_unknown: expected = "unknown"; break;
    case bfd_mach_arm_2:       expected = "armv2"; break;
    case bfd_mach_arm_2a:      expected = "armv2a"; break;
    case bfd_mach_arm_3:       expected = "armv3"; break;
    case bfd_mach_arm_3M:      expected = "armv3M"; break;
    case bfd_mach_arm_4:       expected = "armv4"; break;
    case bfd_mach_arm_4T:      expected = "armv4t"; break;
    case bfd_mach_arm_5:       expected = "armv5"; break;
    case bfd_mach_arm_5T:      expected = "armv5t"; break;
    case bfd_mach_arm_5TE:     expected = "armv5te"; break;
    case bfd_mach_arm_XScale:  expected = "XScale"; break;
    case bfd_mach_arm_iWMMXt:  expected = "iWMMXt"; break;
    case bfd_mach_arm_iWMMXt2: expected = "iWMMXt2"; break;
    }

    /* BUG: If arch_string is not NUL-terminated within buffer_size, strcmp
       will read past the end of the allocated buffer. */
    if (strcmp(arch_string, expected) != 0) {
        /* Not reached before ASan reports OOB read, but keep code close to original. */
        strcpy((char *)buffer + (offsetof(arm_Note, name)
               + ((strlen(NOTE_ARCH_STRING) + 3) & ~3)), expected);
        if (!bfd_set_section_contents(abfd, arm_arch_section, buffer, (file_ptr)0, buffer_size)) {
            _bfd_error_handler("warning: unable to update contents of %s section", note_section);
            goto FAIL;
        }
    }

    free(buffer);
    return true;

FAIL:
    free(buffer);
    return false;
}

int main(void) {
    /* Craft a section whose entire contents are exactly the expected string
       "armv2" without a trailing NUL. This causes strcmp to read past the end
       when checking for the terminator. */
    const char *payload = "armv2"; /* 5 bytes, no trailing NUL copied */
    size_t payload_size = 5; /* intentionally no space for NUL */

    asection *sec = (asection *)calloc(1, sizeof(*sec));
    if (!sec) return 1;
    sec->name = ".note.arm.arch";
    sec->flags = SEC_HAS_CONTENTS;
    sec->size = payload_size;
    sec->contents = (unsigned char *)malloc(payload_size);
    if (!sec->contents) return 1;
    memcpy(sec->contents, payload, payload_size);

    bfd abfd;
    memset(&abfd, 0, sizeof(abfd));
    abfd.sect = sec;

    /* Choose a machine whose expected string is exactly "armv2". */
    abfd.mach = bfd_mach_arm_2;

    /* This call will trigger the out-of-bounds read in strcmp. */
    (void)bfd_arm_update_notes(&abfd);

    /* Cleanup (unreached if ASan aborts). */
    free(sec->contents);
    free(sec);
    return 0;
}
