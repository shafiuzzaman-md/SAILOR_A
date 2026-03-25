#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

/* Minimal, self-contained stubs to mimic the BFD environment around
   bfd_record_phdr with the integer overflow bug. */

typedef uint16_t bfd_size_type; /* Purposely small to demonstrate overflow */
typedef unsigned long flagword;
typedef uint64_t bfd_vma;

/* Forward declaration for asection. */
struct bfd_section { int dummy; };
typedef struct bfd_section asection;

/* Minimal elf_segment_map matching the usage in the vulnerable code. */
struct elf_segment_map {
  struct elf_segment_map *next;
  unsigned long p_type;
  flagword p_flags;
  bfd_vma p_paddr;
  bool p_flags_valid;
  bool p_paddr_valid;
  bool includes_filehdr;
  bool includes_phdrs;
  unsigned int count;
  asection *sections[1]; /* flexible tail (1 + (count-1)) */
};

/* Minimal bfd object holding only what's needed for this reproducer. */
struct bfd {
  int flavour;
  struct elf_segment_map *seg_map_head;
};

enum { bfd_target_elf_flavour = 1 };

#define elf_seg_map(abfd) ((abfd)->seg_map_head)

static inline unsigned int bfd_octets_per_byte(struct bfd *abfd, void *ign)
{
  (void)abfd; (void)ign; return 1; /* bytes == octets */
}

static inline int bfd_get_flavour(struct bfd *abfd)
{
  return abfd->flavour;
}

/* bfd_zalloc takes bfd_size_type (intentionally 16-bit here), which models
   the truncation/overflow issue when large size_t is passed in. */
static void *bfd_zalloc(struct bfd *abfd, bfd_size_type amt)
{
  (void)abfd;
  if (amt == 0) amt = 1; /* avoid malloc(0) undefined behaviors */
  void *p = malloc(amt);
  if (p) memset(p, 0, amt);
  return p;
}

/* Vulnerable function modeled after bfd/bfd.c: bfd_record_phdr. */
bool bfd_record_phdr(struct bfd *abfd,
                     unsigned long type,
                     bool flags_valid,
                     flagword flags,
                     bool at_valid,
                     bfd_vma at,  /* Bytes.  */
                     bool includes_filehdr,
                     bool includes_phdrs,
                     unsigned int count,
                     asection **secs)
{
  struct elf_segment_map *m, **pm;
  size_t amt;
  unsigned int opb = bfd_octets_per_byte(abfd, NULL);

  if (bfd_get_flavour(abfd) != bfd_target_elf_flavour)
    return true;

  /* Integer-overflow-prone size computation (as in the vulnerable code). */
  amt = sizeof(struct elf_segment_map);
  amt += ((bfd_size_type) count - 1) * sizeof(asection *);

  m = (struct elf_segment_map *) bfd_zalloc(abfd, (bfd_size_type) amt);
  if (m == NULL)
    return false;

  m->p_type = type;
  m->p_flags = flags;
  m->p_paddr = at * opb;
  m->p_flags_valid = flags_valid;
  m->p_paddr_valid = at_valid;
  m->includes_filehdr = includes_filehdr;
  m->includes_phdrs = includes_phdrs;
  m->count = count;
  if (count > 0)
    memcpy(m->sections, secs, count * sizeof(asection *));

  for (pm = &elf_seg_map(abfd); *pm != NULL; pm = &(*pm)->next)
    ;
  *pm = m;

  return true;
}

int main(void)
{
  struct bfd abfd;
  abfd.flavour = bfd_target_elf_flavour;
  abfd.seg_map_head = NULL;

  /* Choose count > 65535 so that (bfd_size_type)count (16-bit here) truncates.
     This makes the computed allocation too small, but memcpy still uses the
     full 32-bit 'count' for its size, causing heap-buffer-overflow. */
  const unsigned int count = 70000u; /* ~0.56 MB of pointers on 64-bit */

  asection **secs = (asection **)malloc((size_t)count * sizeof(asection *));
  if (!secs) {
    fprintf(stderr, "Failed to allocate secs\n");
    return 1;
  }

  /* Fill source array with some valid pointers. */
  asection dummy;
  for (unsigned int i = 0; i < count; i++)
    secs[i] = &dummy;

  /* Trigger the bug: Allocation size computed with 16-bit arithmetic due to
     (bfd_size_type) cast, but memcpy uses full 'count'. Under ASan, this
     should report a heap-buffer-overflow in memcpy destination. */
  (void) bfd_record_phdr(&abfd,
                         /*type*/ 1ul,
                         /*flags_valid*/ true,
                         /*flags*/ 0,
                         /*at_valid*/ true,
                         /*at*/ 0,
                         /*includes_filehdr*/ false,
                         /*includes_phdrs*/ false,
                         count,
                         secs);

  /* Prevent optimizing away. */
  if (abfd.seg_map_head)
    printf("Recorded phdr with count=%u\n", abfd.seg_map_head->count);

  free(secs);
  return 0;
}
