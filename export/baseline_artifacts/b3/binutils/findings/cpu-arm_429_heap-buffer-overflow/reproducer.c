#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stddef.h>

/* Minimal stubs and types to simulate the vulnerable code path in
   bfd/cpu-arm.c:bfd_arm_update_notes. */

typedef long file_ptr;

/* Section flags */
#define SEC_HAS_CONTENTS 0x1

/* Machine enums (subset) */
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

/* Minimal asection and bfd stand-ins. */
typedef struct asection {
  const char *name;
  unsigned long flags;
  unsigned long size;
  void *contents;
} asection;

typedef struct bfd {
  unsigned int mach;
  asection *only_section; /* Simplify: single section */
} bfd;

/* NOTE name used by the code for offset calculation. */
#define NOTE_ARCH_STRING "arch"

/* The ELF note header used by the ARM note parser in BFD. */
typedef struct {
  uint32_t namesz;
  uint32_t descsz;
  uint32_t type;
  char name[1]; /* name (null-terminated), then 4-byte padded; desc follows */
} arm_Note;

/* Stubs matching BFD API used by bfd_arm_update_notes. */
static asection *bfd_get_section_by_name(bfd *abfd, const char *name) {
  if (!abfd || !abfd->only_section) return NULL;
  if (abfd->only_section->name && name && strcmp(abfd->only_section->name, name) == 0)
    return abfd->only_section;
  return NULL;
}

static bool bfd_malloc_and_get_section(bfd *abfd, asection *sec, void **out) {
  (void)abfd;
  if (!sec || !out) return false;

  /* Craft a note with descsz == 0 and buffer_size == header + aligned(namesz) + aligned(descsz).
     namesz includes the null terminator, which will make aligned(namesz) > aligned(strlen(NOTE_ARCH_STRING)). */
  const size_t name_len = strlen(NOTE_ARCH_STRING);
  const uint32_t namesz = (uint32_t)(name_len + 1); /* include NUL */
  const uint32_t descsz = 0; /* critical for overflow */

  const size_t name_aligned = (namesz + 3u) & ~3u;
  const size_t desc_aligned = (descsz + 3u) & ~3u; /* = 0 */
  const size_t base = offsetof(arm_Note, name);
  const size_t total = base + name_aligned + desc_aligned; /* descsz == 0 */

  sec->size = (unsigned long) total;
  unsigned char *buf = (unsigned char *)malloc(total);
  if (!buf) return false;
  memset(buf, 0, total);

  arm_Note *note = (arm_Note *)buf;
  note->namesz = namesz;
  note->descsz = descsz;
  note->type = 0;
  memcpy(note->name, NOTE_ARCH_STRING, name_len);
  note->name[name_len] = '\0';
  /* padding already zeroed */

  *out = buf;
  return true;
}

/* Return a mismatching architecture string so strcmp != 0 and the strcpy path executes. */
static bool arm_check_note(bfd *abfd, unsigned char *buffer, size_t buffer_size,
                           const char *note_name, const char **arch_string_out) {
  (void)abfd; (void)buffer; (void)buffer_size; (void)note_name;
  if (!arch_string_out) return false;
  *arch_string_out = "not-expected"; /* ensure mismatch with expected */
  return true;
}

static unsigned int bfd_get_mach(bfd *abfd) {
  return abfd ? abfd->mach : bfd_mach_arm_unknown;
}

static bool bfd_set_section_contents(bfd *abfd, asection *sec, const void *buf,
                                     file_ptr offset, size_t count) {
  (void)abfd; (void)sec; (void)buf; (void)offset; (void)count;
  /* In the real code this would write back into the object; here just succeed. */
  return true;
}

/* Dummy i18n macro and error handler to satisfy references (won't be used). */
#define _(X) (X)
static void _bfd_error_handler(const char *fmt, ...) { (void)fmt; }

/* The vulnerable function (reproduced minimally). */
static bool bfd_arm_update_notes(bfd *abfd, const char *note_section) {
  asection *arm_arch_section;
  unsigned long buffer_size;
  unsigned char *buffer = NULL;
  const char *arch_string = NULL;
  const char *expected = NULL;

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

  /* Determine expected string from machine. */
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

  if (strcmp(arch_string, expected) != 0) {
    /* Vulnerable write: does not verify descsz or total buffer_size. */
    strcpy((char *)buffer + (offsetof(arm_Note, name)
                              + ((strlen(NOTE_ARCH_STRING) + 3) & ~3)),
           expected);

    if (!bfd_set_section_contents(abfd, arm_arch_section, buffer, (file_ptr)0, buffer_size)) {
      _bfd_error_handler(_("warning: unable to update contents of %s section in %pB"),
                         note_section, abfd);
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
  /* Set up a fake BFD and a single section with contents. */
  asection sec;
  sec.name = ".note.arm.arch"; /* arbitrary name, must match lookup */
  sec.flags = SEC_HAS_CONTENTS;
  sec.size = 16; /* initial; real size set by bfd_malloc_and_get_section */
  sec.contents = NULL;

  bfd abfd;
  abfd.mach = bfd_mach_arm_unknown; /* expected == "unknown" */
  abfd.only_section = &sec;

  /* Trigger the vulnerable path. With descsz == 0 and exact buffer size,
     the strcpy will overflow the heap buffer allocated in bfd_malloc_and_get_section. */
  bool ok = bfd_arm_update_notes(&abfd, sec.name);
  printf("bfd_arm_update_notes returned: %s\n", ok ? "true" : "false");
  return 0;
}
