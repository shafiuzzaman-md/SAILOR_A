// Standalone reproducer for heap-buffer-overflow in mips_relocate_hi
// Triggered by out-of-bounds write via bfd_put_32 at
// contents + (refhi->r_vaddr - input_section->vma)

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

// Minimal type re-declarations to mirror BFD interfaces used
typedef unsigned long bfd_vma;    // Matches common BFD usage on 64-bit hosts
typedef unsigned char bfd_byte;

typedef struct bfd {
  int dummy;
} bfd;

typedef struct asection {
  bfd_vma vma;   // Section load address (base for relocations)
} asection;

typedef struct internal_reloc {
  bfd_vma r_vaddr;   // Virtual address of the relocation within the section
} internal_reloc;

// Stubbed BFD accessors
// Make bfd_get_32 a macro that does NOT touch the provided pointer, so we avoid
// an earlier OOB-read and only trigger on the write (matching the report focus).
#define bfd_get_32(abfd, ptr) (0U)

// Make bfd_put_32 a macro that performs a 4-byte write at the provided pointer.
// Using a macro makes the write happen at the call site, closely matching the bug site.
#define bfd_put_32(abfd, val, ptr) do {            \
  unsigned char *p__ = (unsigned char *)(ptr);     \
  unsigned int v__ = (unsigned int)(val);          \
  /* Write 4 bytes (big-endian order is arbitrary here; any write will do). */ \
  p__[0] = (unsigned char)((v__ >> 24) & 0xFF);    \
  p__[1] = (unsigned char)((v__ >> 16) & 0xFF);    \
  p__[2] = (unsigned char)((v__ >> 8) & 0xFF);     \
  p__[3] = (unsigned char)(v__ & 0xFF);            \
} while (0)

// Vulnerable function (adapted from bfd/coff-mips.c)
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

  insn = bfd_get_32(input_bfd, contents + refhi->r_vaddr - input_section->vma);
  if (reflo == NULL)
    vallo = 0;
  else
    vallo = (bfd_get_32(input_bfd, contents + reflo->r_vaddr - input_section->vma) & 0xffff);

  val = ((insn & 0xffff) << 16) + vallo;
  val += relocation;

  if ((vallo & 0x8000) != 0)
    val -= 0x10000;

  if ((val & 0x8000) != 0)
    val += 0x10000;

  insn = (insn & ~(unsigned)0xffff) | ((val >> 16) & 0xffff);
  // Out-of-bounds write: no bounds check on (refhi->r_vaddr - input_section->vma)
  bfd_put_32(input_bfd, (bfd_vma)insn, contents + refhi->r_vaddr - input_section->vma);
}

int main(void) {
  // Prepare a small heap buffer to represent section contents
  size_t contents_size = 16; // small buffer to make OOB easy to hit
  bfd_byte *contents = (bfd_byte *)malloc(contents_size);
  if (!contents) {
    perror("malloc");
    return 1;
  }
  memset(contents, 0xAA, contents_size);

  // Set up fake BFD and section
  bfd fake_bfd = {0};
  asection input_section = {0};
  // Keep vma at 0 so the offset is exactly r_vaddr
  input_section.vma = 0;

  // Craft relocation entries. We only need refhi; reflo can be NULL.
  internal_reloc refhi;
  // Choose an address well beyond the end of the contents buffer.
  // This ensures contents + (refhi.r_vaddr - vma) points OOB.
  refhi.r_vaddr = (bfd_vma)(contents_size + 32); // definitely OOB

  // Trigger the vulnerable path; relocation delta is irrelevant for OOB
  mips_relocate_hi(&refhi, NULL, &fake_bfd, &input_section, contents, 0);

  // Cleanup (we likely won't reach here cleanly under ASan)
  free(contents);
  return 0;
}
