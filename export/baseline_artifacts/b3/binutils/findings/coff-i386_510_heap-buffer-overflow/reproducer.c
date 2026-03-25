#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Minimal stand-ins for BFD types used in the vulnerable code path. */
typedef uint64_t bfd_vma;
typedef unsigned char bfd_byte;
typedef int bfd_boolean;

struct bfd {}; /* Unused in this reproducer */
struct bfd_link_info {}; /* Unused in this reproducer */

typedef struct asection {
  bfd_vma vma;
  bfd_vma size;
  struct asection *output_section;
  struct asection *next;
} asection;

struct internal_reloc {
  bfd_vma r_vaddr; /* relocation target VA */
  int r_symndx;
  unsigned short r_type;
};

struct internal_syment {}; /* Unused */

/* Little-endian 16-bit write like bfd_putl16. */
static inline void bfd_putl16(unsigned int val, bfd_byte *p) {
  p[0] = (bfd_byte)(val & 0xff);
  p[1] = (bfd_byte)((val >> 8) & 0xff);
}

/* Stub for _bfd_coff_generic_relocate_section; not used meaningfully here. */
static bfd_boolean _bfd_coff_generic_relocate_section(
    struct bfd *output_bfd,
    struct bfd_link_info *info,
    struct bfd *input_bfd,
    asection *input_section,
    bfd_byte *contents,
    struct internal_reloc *relocs,
    struct internal_syment *syms,
    asection **sections) {
  (void)output_bfd; (void)info; (void)input_bfd; (void)input_section;
  (void)contents; (void)relocs; (void)syms; (void)sections;
  return 1;
}

/* Simplified version of coff_pe_i386_relocate_section that preserves the
 * vulnerable write at: contents + (rel->r_vaddr - input_section->vma).
 * This reproducer intentionally omits the missing bounds check to demonstrate
 * the heap-buffer-overflow.
 */
bfd_boolean coff_pe_i386_relocate_section(
    struct bfd *output_bfd,
    struct bfd_link_info *info,
    struct bfd *input_bfd,
    asection *input_section,
    bfd_byte *contents,
    struct internal_reloc *relocs,
    struct internal_syment *syms,
    asection **sections) {
  (void)output_bfd; (void)info; (void)input_bfd; (void)syms; (void)sections;

  /* In the real code, idx is found by walking output_bfd->sections to find
   * the output section index of the referenced section. Any small value will do. */
  unsigned int idx = 0;

  /* Trigger the vulnerable write for the first relocation entry. */
  struct internal_reloc *rel = &relocs[0];

  /* Vulnerable out-of-bounds write (no validation that r_vaddr is within the
   * [input_section->vma, input_section->vma + input_section->size) range). */
  bfd_putl16(idx, contents + (size_t)(rel->r_vaddr - input_section->vma));

  return _bfd_coff_generic_relocate_section(output_bfd, info, input_bfd,
                                            input_section, contents,
                                            relocs, syms, sections);
}

int main(void) {
  /* Set up a tiny section buffer to make ASan detect an OOB quickly. */
  const size_t sec_size = 16;
  bfd_byte *contents = (bfd_byte *)malloc(sec_size);
  if (!contents) {
    perror("malloc");
    return 1;
  }
  memset(contents, 0xAA, sec_size);

  /* Create an input section with some base vma. */
  asection input_sec;
  input_sec.vma = 0x1000;
  input_sec.size = sec_size;
  input_sec.output_section = NULL;
  input_sec.next = NULL;

  /* Craft a relocation whose r_vaddr points outside the input section's bounds.
   * This ensures contents + (r_vaddr - vma) points past the heap buffer. */
  struct internal_reloc rel;
  rel.r_vaddr = input_sec.vma + sec_size + 8; /* 8 bytes past the end */
  rel.r_symndx = 0;
  rel.r_type = 0;

  /* Unused placeholders to satisfy function signature. */
  struct bfd out_bfd_obj, in_bfd_obj;
  struct bfd_link_info link_info;
  struct internal_syment symtab_dummy;
  asection *sections_dummy = NULL;

  /* Call the vulnerable function. Under ASan, this should report a
   * heap-buffer-overflow when it performs the 16-bit write out-of-bounds. */
  (void)coff_pe_i386_relocate_section(&out_bfd_obj, &link_info, &in_bfd_obj,
                                      &input_sec, contents, &rel,
                                      &symtab_dummy, &sections_dummy);

  /* If we somehow didn't crash, clean up. */
  free(contents);
  printf("Completed without ASan report (unexpected).\n");
  return 0;
}
