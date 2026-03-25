#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <stdint.h>

/* Minimal re-declarations to exercise the vulnerable code path. */

typedef size_t bfd_size_type;

/* Classic Unix ar header layout. */
struct ar_hdr {
  char ar_name[16];
  char ar_date[12];
  char ar_uid[6];
  char ar_gid[6];
  char ar_mode[8];
  char ar_size[10];
  char ar_fmag[2];
};

struct areltdata {
  char *arch_header;      /* Points to an ar_hdr blob. */
  bfd_size_type parsed_size;
};

typedef struct bfd {
  struct areltdata *arelt_data;
} bfd;

/* Stub bfd error API used by the function (won't be hit in this reproducer). */
enum bfd_error {
  bfd_error_invalid_operation = 1
};
static void bfd_set_error(enum bfd_error e) { (void)e; }

/* Helpers mirroring what libbfd uses. */
static inline struct ar_hdr *arch_hdr(bfd *abfd) {
  if (!abfd || !abfd->arelt_data) return NULL;
  return (struct ar_hdr *) abfd->arelt_data->arch_header;
}
#define arch_eltdata(abfd) ((abfd)->arelt_data)

/* Vulnerable function copied/trimmed to the essentials for the reproducer. */
int bfd_generic_stat_arch_elt(bfd *abfd, struct stat *buf) {
  struct ar_hdr *hdr;
  char *aloser;

  if (abfd->arelt_data == NULL) {
    bfd_set_error(bfd_error_invalid_operation);
    return -1;
  }

  hdr = arch_hdr(abfd);
  if (hdr == NULL)
    return -1;

#define foo(arelt, stelt, size)                 \
  buf->stelt = strtol(hdr->arelt, &aloser, size); \
  if (aloser == hdr->arelt)                    \
    return -1;

#define foo2(arelt, stelt, size) foo(arelt, stelt, size)

  /* These calls parse fixed-width, space-padded fields without a NUL or length bound. */
  foo (ar_date, st_mtime, 10);
  foo2(ar_uid,  st_uid,   10);
  foo2(ar_gid,  st_gid,   10);
  foo (ar_mode, st_mode,   8);

  buf->st_size = arch_eltdata(abfd)->parsed_size;
  return 0;
}

int main(void) {
  /* Allocate an ar_hdr object in its own heap chunk so ASan places a redzone
     immediately after it. Fill the entire header with digit characters so
     strtol keeps scanning across field boundaries and into the redzone. */
  struct ar_hdr *hdr = (struct ar_hdr *)malloc(sizeof(struct ar_hdr));
  if (!hdr) {
    perror("malloc");
    return 1;
  }
  memset(hdr, '7', sizeof(*hdr)); /* All digits, no terminating NULs or spaces */

  /* Build minimal bfd/areltdata to point to our crafted header. */
  struct areltdata *ared = (struct areltdata *)malloc(sizeof(struct areltdata));
  if (!ared) {
    perror("malloc");
    return 1;
  }
  ared->arch_header = (char *)hdr;
  ared->parsed_size = 123; /* arbitrary */

  bfd abfd; memset(&abfd, 0, sizeof(abfd));
  abfd.arelt_data = ared;

  struct stat st; memset(&st, 0, sizeof(st));

  /* This call should trigger an out-of-bounds read inside strtol during the
     first foo(ar_date, ...) expansion because it will continue scanning past
     the end of ar_date, through subsequent fields, and beyond the struct end
     into the ASan redzone. */
  (void)bfd_generic_stat_arch_elt(&abfd, &st);

  /* If the bug does not trigger (unexpected), clean up. */
  free(ared);
  free(hdr);
  return 0;
}
