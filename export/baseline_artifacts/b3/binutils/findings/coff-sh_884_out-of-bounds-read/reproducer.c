#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Minimal stand-ins for BFD types. */
typedef struct bfd bfd; /* Opaque in real BFD; unused here. */

typedef struct asection {
  uint64_t vma;            /* Virtual memory address of section start. */
  size_t size;             /* Size of section contents in bytes. */
  struct asection *output_section;
  uint64_t output_offset;
  int target_index;
} asection;

/* Minimal bfd_get_32 that performs a 4-byte read from the provided address. */
static inline uint32_t bfd_get_32(bfd *abfd, const unsigned char *p) {
  (void)abfd; /* Endianness is irrelevant for triggering the bug. */
  uint32_t v;
  /* This memcpy will read 4 bytes from p, even if it crosses the buffer end. */
  memcpy(&v, p, 4);
  return v;
}

/* Simplified version of the vulnerable code path from sh_relax_section.
   It reproduces the faulty check and subsequent out-of-bounds 32-bit read:
     symval += bfd_get_32 (abfd, contents + paddr - sec->vma);
   after only verifying paddr < sec->size (and not ensuring 4 bytes remain). */
static void trigger_oob_read(bfd *abfd, asection *sec, unsigned char *contents, uint64_t paddr) {
  /* The real code validates only that paddr < sec->size. */
  if (paddr >= sec->size) {
    printf("paddr out of range, not triggering\n");
    return;
  }

  /* Vulnerable 4-byte read that may run past the end of contents. */
  uint64_t symval = 0;
  symval += bfd_get_32(abfd, contents + (paddr - sec->vma));

  /* Use symval so the read isn't optimized away. */
  volatile uint64_t sink = symval;
  (void)sink;
}

int main(void) {
  /* Set up a tiny fake section with contents. */
  const size_t sz = 16;                      /* Section size. */
  unsigned char *contents = (unsigned char *)malloc(sz);
  if (!contents) {
    perror("malloc");
    return 1;
  }
  /* Fill with some bytes. */
  for (size_t i = 0; i < sz; i++) contents[i] = (unsigned char)i;

  asection sec;
  memset(&sec, 0, sizeof(sec));
  sec.vma = 0;                               /* Simplify: paddr is an offset. */
  sec.size = sz;                             /* Contents buffer length. */
  sec.output_section = &sec;                 /* Not used but mirrors structure. */
  sec.output_offset = 0;
  sec.target_index = 1;

  bfd fake_bfd_obj;                          /* Opaque; unused. */

  /* Choose paddr so that the guard (paddr < sec->size) passes, but there are
     fewer than 4 bytes remaining, causing a 4-byte read to run past the end.
     For sz == 16, paddr == 15 will read 3 bytes past the allocation. */
  uint64_t paddr = sz - 1;                   /* Triggers OOB read by 3 bytes. */

  printf("Triggering OOB 4-byte read at offset %llu (size=%zu) ...\n",
         (unsigned long long)paddr, sz);
  trigger_oob_read(&fake_bfd_obj, &sec, contents, paddr);

  /* If ASan is enabled, it should report a heap-buffer-overflow (READ) here. */
  free(contents);
  return 0;
}
