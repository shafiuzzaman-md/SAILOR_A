#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Minimal stand-ins for BFD types/structures used by coff-alpha.c */
typedef uint64_t bfd_vma;

typedef struct reloc_howto_struct {
  int pc_relative;
  int size; /* bytes written by relocation */
} reloc_howto_type;

typedef struct asection_struct {
  bfd_vma vma;
  bfd_vma output_offset;
  struct asection_struct *output_section;
} asection;

/* Stub for _bfd_relocate_contents that writes to the provided pointer. */
static int _bfd_relocate_contents(const reloc_howto_type *howto,
                                  void *input_bfd,
                                  bfd_vma relocation,
                                  void *where)
{
  /* Simulate a relocation write by storing 'size' bytes at 'where'. */
  (void)input_bfd; /* unused in this stub */
  unsigned char *p = (unsigned char *)where;
  /* Write a recognizable pattern to trigger ASan on OOB. */
  int bytes = howto && howto->size > 0 ? howto->size : 8;
  for (int i = 0; i < bytes; i++) {
    p[i] = (unsigned char)(0xA0 + (relocation + i) % 0x1F);
  }
  return 0; /* bfd_reloc_ok in real code */
}

/* This function mimics the vulnerable call site in coff-alpha.c:alpha_relocate_section.
 * It computes the relocation write address as contents + (r_vaddr - input_section->vma)
 * without validating that r_vaddr is within the section bounds, then calls
 * _bfd_relocate_contents to perform the write. */
static void vulnerable_alpha_relocate_section(unsigned char *contents,
                                              size_t contents_size,
                                              bfd_vma r_vaddr,
                                              const asection *input_section)
{
  reloc_howto_type howto;
  howto.pc_relative = 0;
  howto.size = 8; /* write 8 bytes, similar to a 64-bit relocation */

  bfd_vma relocation = 0x12345678; /* arbitrary addend/result */

  /* Vulnerable address computation (mirrors the real bug): */
  unsigned char *where = contents + (r_vaddr - input_section->vma);

  /* This call will perform an out-of-bounds write if r_vaddr falls
   * outside [input_section->vma, input_section->vma + contents_size). */
  (void)_bfd_relocate_contents(&howto, NULL, relocation, (void *)where);
}

int main(void)
{
  /* Allocate a small heap buffer that represents the section contents. */
  const size_t section_size = 16; /* small to make OOB easier to hit redzone */
  unsigned char *contents = (unsigned char *)malloc(section_size);
  if (!contents) {
    perror("malloc");
    return 1;
  }
  memset(contents, 0, section_size);

  /* Set up a fake input section with some VMA. */
  asection input_section;
  input_section.vma = 0x4000; /* arbitrary non-zero VMA */
  input_section.output_offset = 0;
  input_section.output_section = NULL;

  /* Choose an r_vaddr just past the end of the section so that:
   *   where = contents + (r_vaddr - input_section->vma) = contents + section_size
   * which lands in the heap redzone and triggers a heap-buffer-overflow under ASan. */
  bfd_vma r_vaddr = input_section.vma + section_size; /* exact end => OOB for an 8-byte write */

  /* Trigger the vulnerable path. */
  vulnerable_alpha_relocate_section(contents, section_size, r_vaddr, &input_section);

  /* Clean up. The ASan report should have already been produced by now. */
  free(contents);
  return 0;
}
