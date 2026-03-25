#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* Minimal stub types to mirror the required BFD interfaces. */
typedef struct bfd { int dummy; } bfd;
typedef unsigned char bfd_byte;
typedef size_t bfd_vma;

typedef struct asection {
  bfd_vma vma;
} asection;

struct internal_reloc {
  bfd_vma r_vaddr;
};

/* Stub implementations that perform raw 4-byte memory accesses. */
static unsigned long bfd_get_32(bfd *abfd, const void *p) {
  (void)abfd;
  uint32_t v;
  /* This memcpy will read 4 bytes from p; if p is out of bounds, ASan will flag it. */
  memcpy(&v, p, sizeof(v));
  return (unsigned long)v;
}

static void bfd_put_32(bfd *abfd, bfd_vma val, void *p) {
  (void)abfd;
  uint32_t v = (uint32_t)val;
  /* This memcpy will write 4 bytes to p; if p is out of bounds, ASan will flag it. */
  memcpy(p, &v, sizeof(v));
}

/* Direct copy of the vulnerable logic from bfd/coff-mips.c (simplified context). */
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

  /* Vulnerable OOB read when refhi->r_vaddr is outside the section buffer. */
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
  /* This will also perform an OOB write back to the same invalid location. */
  bfd_put_32(input_bfd, (bfd_vma)insn,
             contents + refhi->r_vaddr - input_section->vma);
}

int main(void) {
  /* Create a tiny fake section buffer. */
  size_t sec_size = 16;
  bfd_byte *contents = (bfd_byte *)malloc(sec_size);
  if (!contents) {
    perror("malloc");
    return 1;
  }
  for (size_t i = 0; i < sec_size; i++) contents[i] = (bfd_byte)(i & 0xFF);

  /* Set up a fake section and relocations. */
  asection input_section;
  input_section.vma = 0x1000;  /* Arbitrary base VMA. */

  struct internal_reloc refhi;
  /* Point r_vaddr one byte past the end of the section buffer to trigger OOB. */
  refhi.r_vaddr = input_section.vma + sec_size + 1;

  /* reflo is not needed to trigger the first OOB read. */
  struct internal_reloc *reflo = NULL;

  bfd *input_bfd = NULL;  /* Not used by our stubs. */
  bfd_vma relocation = 0;

  /* This call should trigger an out-of-bounds read in bfd_get_32 via mips_relocate_hi. */
  mips_relocate_hi(&refhi, reflo, input_bfd, &input_section, contents, relocation);

  /* If ASan didn't abort yet, clean up. */
  free(contents);
  return 0;
}
