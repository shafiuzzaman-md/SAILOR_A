#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

/* Minimal stand-ins for BFD types used by xcoff_write_armap_big. */
typedef struct bfd_arch_info_type {
  int bits_per_address;
} bfd_arch_info_type;

typedef struct bfd {
  struct bfd *archive_next;   /* Next member in archive list. */
  struct bfd *archive_head;   /* Head of the member list (only meaningful on the top-level abfd). */
  const bfd_arch_info_type *arch_info; /* Architecture info. */
} bfd;

struct orl {
  bfd *abfd;
  const char **name; /* In the real code, name is a char**; emulate enough for strlen(*name). */
};

static const bfd_arch_info_type *bfd_get_arch_info(bfd *abfd) {
  return abfd->arch_info;
}

/* A reduced copy of the vulnerable logic from bfd/coff-rs6000.c:xcoff_write_armap_big.
   It intentionally preserves the bug: the inner while() lacks a bound check on i. */
static bool xcoff_write_armap_big(bfd *abfd, unsigned int elength,
                                  struct orl *map, unsigned int orl_count, int stridx) {
  (void)elength;
  (void)stridx;

  size_t string_length;
  const bfd_arch_info_type *arch_info;
  bfd *current_bfd;
  unsigned long long i, sym_32 = 0, sym_64 = 0, str_32 = 0, str_64 = 0;

  i = 0;
  for (current_bfd = abfd->archive_head;
       current_bfd != NULL && i < orl_count;
       current_bfd = current_bfd->archive_next) {
    arch_info = bfd_get_arch_info(current_bfd);
    /* Vulnerable condition: no check that i < orl_count before dereferencing map[i]. */
    while (map[i].abfd == current_bfd) {
      string_length = strlen(*map[i].name) + 1;
      if (arch_info->bits_per_address == 64) {
        sym_64++;
        str_64 += string_length;
      } else {
        sym_32++;
        str_32 += string_length;
      }
      i++;
    }
  }

  /* The function would normally continue, but for this reproducer we return here. */
  return true;
}

int main(void) {
  /* Set up two archive members (m1 -> m2). */
  bfd_arch_info_type arch32 = { .bits_per_address = 32 };
  bfd_arch_info_type arch64 = { .bits_per_address = 64 };

  bfd m1 = { .archive_next = NULL, .archive_head = NULL, .arch_info = &arch32 };
  bfd m2 = { .archive_next = NULL, .archive_head = NULL, .arch_info = &arch64 };
  m1.archive_next = &m2; /* m1 -> m2 */

  /* Top-level archive bfd whose archive_head points to first member. */
  bfd archive = { .archive_next = NULL, .archive_head = &m1, .arch_info = &arch32 };

  /* Build an orl map where the final group belongs to m2 and exactly reaches the end.
     This causes i to become orl_count while still inside the inner while, leading to
     an out-of-bounds read on the next condition check (map[i].abfd). */
  const unsigned int orl_count = 3;
  struct orl *map = (struct orl *)malloc(orl_count * sizeof(struct orl));
  if (!map) {
    perror("malloc");
    return 1;
  }

  const char *n0 = "A";
  const char *n1 = "B";
  const char *n2 = "C";

  /* Entries: index 0 belongs to m1; indices 1 and 2 belong to m2. */
  map[0].abfd = &m1; map[0].name = &n0;
  map[1].abfd = &m2; map[1].name = &n1;
  map[2].abfd = &m2; map[2].name = &n2;

  /* This call triggers the OOB read at the while condition when i == orl_count. */
  (void)xcoff_write_armap_big(&archive, 0, map, orl_count, 0);

  /* If ASan did not abort yet, clean up. */
  free(map);
  puts("Done (AddressSanitizer should have reported an out-of-bounds read above).");
  return 0;
}
