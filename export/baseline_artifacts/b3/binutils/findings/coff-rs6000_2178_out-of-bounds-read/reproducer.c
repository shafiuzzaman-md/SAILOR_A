#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Minimal stub types to mirror the relevant pieces used by the buggy code. */
typedef struct bfd_arch_info_type {
  int bits_per_address; /* 32 or 64 */
} bfd_arch_info_type;

typedef struct bfd bfd;
struct bfd {
  bfd *archive_head;
  bfd *archive_next;
  bfd_arch_info_type arch; /* Per-member arch info */
  const char *name;
};

/* orl map entry: associates an archive member with a symbol name pointer. */
typedef struct orl_entry {
  bfd *abfd;           /* Archive member this symbol comes from */
  const char **name;   /* Pointer to a C-string pointer (matches *map[i].name usage) */
} orl_entry;

/* Stub for bfd_get_arch_info used by the vulnerable code. */
static bfd_arch_info_type *bfd_get_arch_info(bfd *abfd) {
  return &abfd->arch;
}

/* This function reproduces the vulnerable loop from bfd/coff-rs6000.c:xcoff_write_armap_big
   focusing on the 32-bit symbol names section that contains the OOB read:

     i = 0;
     for (current_bfd = abfd->archive_head;
          current_bfd != NULL && i < orl_count;
          current_bfd = current_bfd->archive_next)
     {
       arch_info = bfd_get_arch_info (current_bfd);
       while (map[i].abfd == current_bfd) {
         if (arch_info->bits_per_address == 32) {
           string_length = sprintf (st, "%s", *map[i].name);
           st += string_length + 1;
         }
         i++;
       }
     }

   The inner while condition reads map[i] again after i++ without re-checking i < orl_count,
   causing an out-of-bounds read when i becomes orl_count.
*/
static int xcoff_write_armap_big(bfd *abfd, orl_entry *map, size_t orl_count) {
  char *symbol_table = (char *)malloc(1024);
  if (!symbol_table) return 0;
  char *st = symbol_table;
  size_t i = 0;
  bfd *current_bfd;
  bfd_arch_info_type *arch_info;
  int string_length;

  /* Vulnerable 32-bit symbol names loop */
  for (current_bfd = abfd->archive_head;
       current_bfd != NULL && i < orl_count;
       current_bfd = current_bfd->archive_next)
  {
    arch_info = bfd_get_arch_info(current_bfd);
    /* BUG: missing bounds check on i inside this loop */
    while (map[i].abfd == current_bfd) {
      if (arch_info->bits_per_address == 32) {
        string_length = sprintf(st, "%s", *map[i].name);
        st += string_length + 1;
      }
      /* i can become == orl_count, and the next evaluation of map[i] reads past end */
      i++;
    }
  }

  free(symbol_table);
  return 1;
}

int main(void) {
  /* Set up a minimal archive with a single member having 32-bit addresses. */
  bfd archive = {0};
  bfd member1 = {0};
  member1.arch.bits_per_address = 32;  /* Ensure body executes */
  archive.archive_head = &member1;     /* Single-member archive */
  member1.archive_next = NULL;

  /* Craft map with exactly one entry so that i becomes orl_count (=1) inside the inner while,
     then the condition re-check performs map[1].abfd (OOB read). */
  size_t orl_count = 1;
  orl_entry *map = (orl_entry *)malloc(sizeof(*map) * orl_count);
  if (!map) {
    fprintf(stderr, "allocation failure\n");
    return 1;
  }

  const char *sym0 = "SYM0";
  map[0].abfd = &member1;   /* Matches current_bfd for first (and only) member */
  map[0].name = &sym0;      /* Matches usage sprintf(st, "%s", *map[i].name) */

  /* Trigger the vulnerable function: causes OOB read on map[1].abfd when i increments to 1. */
  (void)xcoff_write_armap_big(&archive, map, orl_count);

  free(map);
  return 0;
}
