#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

/* Minimal stand-ins for BFD types. */
typedef unsigned char bfd_byte;
typedef unsigned long bfd_vma; /* matches typical BFD usage on 64-bit */

typedef struct { /* opaque in this reproducer */
  int dummy;
} bfd;

typedef struct {
  bfd_vma vma;
} asection;

struct internal_reloc {
  bfd_vma r_vaddr; /* relocation address (virtual address) */
};

/* Minimal bfd_get_32/bfd_put_32 that operate on raw buffers. */
static unsigned long bfd_get_32(bfd *abfd, const bfd_byte *p) {
  (void)abfd; /* unused in this stub */
  /* Deliberately perform a 4-byte read from p. If p is out-of-bounds,
     ASan will flag a heap-buffer-overflow. */
  uint32_t v;
  memcpy(&v, p, sizeof(v));
  return (unsigned long)v;
}

static void bfd_put_32(bfd *abfd, bfd_vma val, bfd_byte *p) {
  (void)abfd; /* unused */
  uint32_t v = (uint32_t)val;
  memcpy(p, &v, sizeof(v));
}

/* The vulnerable function (trimmed to the essential logic shown in the report). */
static void mips_relocate_hi(struct internal_reloc *refhi,
                             struct internal_reloc *reflo,
                             bfd *input_bfd,
                             asection *input_section,
                             bfd_byte *contents,
                             bfd_vma relocation)
{
  unsigned long insn;
  unsigned long val;
  unsigned long vallo;

  if (refhi == NULL)
    return;

  insn = bfd_get_32(input_bfd,
                    contents + refhi->r_vaddr - input_section->vma);
  if (reflo == NULL)
    vallo = 0;
  else
    vallo = (bfd_get_32(input_bfd,
                        contents + reflo->r_vaddr - input_section->vma)
             & 0xffff);

  val = ((insn & 0xffff) << 16) + vallo;
  val += relocation;

  if ((vallo & 0x8000) != 0)
    val -= 0x10000;

  if ((val & 0x8000) != 0)
    val += 0x10000;

  insn = (insn & ~(unsigned)0xffff) | ((val >> 16) & 0xffff);
  bfd_put_32(input_bfd, (bfd_vma)insn,
             contents + refhi->r_vaddr - input_section->vma);
}

int main(void) {
  /* Allocate a small contents buffer to simulate a section's contents. */
  size_t contents_size = 16; /* Small buffer to make OOB obvious */
  bfd_byte *contents = (bfd_byte *)malloc(contents_size);
  if (!contents) {
    perror("malloc");
    return 1;
  }
  memset(contents, 0xAA, contents_size);

  /* Set up fake BFD and section. */
  bfd fake_bfd = {0};
  asection input_section = {0}; /* vma = 0 */

  /* Prepare relocations:
     - refhi within bounds so the first read succeeds.
     - reflo deliberately out-of-bounds to trigger the OOB read at line 772.
  */
  struct internal_reloc refhi = { .r_vaddr = 0 };        /* points to start of contents */
  struct internal_reloc reflo = { .r_vaddr = 0x1000 };   /* far beyond contents_size */

  /* Initialize the first 4 bytes so the first read is valid. */
  uint32_t hi_word = 0x12345678;
  memcpy(contents + refhi.r_vaddr - input_section.vma, &hi_word, sizeof(hi_word));

  /* Calling this should perform an out-of-bounds 4-byte read from
     contents + (reflo.r_vaddr - input_section.vma), which ASan will catch. */
  mips_relocate_hi(&refhi, &reflo, &fake_bfd, &input_section, contents, 0);

  /* If it somehow didn't crash (it should under ASan), clean up. */
  free(contents);
  puts("Completed without ASan catching OOB (unexpected).\n");
  return 0;
}