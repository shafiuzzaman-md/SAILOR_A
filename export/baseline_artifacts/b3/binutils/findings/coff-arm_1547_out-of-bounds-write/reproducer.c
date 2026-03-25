#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Minimal stand-ins for BFD types used by coff_arm_relocate_section. */
typedef uint64_t bfd_vma;
typedef int64_t  bfd_signed_vma;

typedef struct bfd {
  int dummy;
} bfd;

typedef struct asection {
  bfd_vma vma;            /* VMA of the input section */
  bfd_vma output_offset;  /* Unused here, but present in real code */
  struct outsec {         /* Unused here */
    bfd_vma vma;
  } *output_section;
} asection;

typedef struct reloc_entry {
  bfd_vma r_vaddr;  /* relocation virtual address (offset within section) */
} reloc_entry;

/* Stubs mimicking BFD helpers. */
static inline uint32_t insert_thumb_branch(uint32_t insn, bfd_signed_vma ret_offset) {
  /* The precise details don't matter; we just produce some 32-bit value. */
  (void)ret_offset;
  return insn ^ 0x12345678u;
}

/* Intentionally do NOT read from 'where' to avoid an earlier OOB read. */
static inline uint32_t bfd_get_32(bfd *abfd, const void *where) {
  (void)abfd; (void)where;
  return 0xDEADBEEFu;
}

/* Perform a 32-bit little-endian store at the target address. */
static inline void bfd_put_32(bfd *abfd, bfd_vma val, void *where) {
  (void)abfd;
  unsigned char *p = (unsigned char *)where;
  /* This write will be out-of-bounds when 'where' points past the end
     of the section buffer, which is the essence of the original bug. */
  p[0] = (unsigned char)(val & 0xFF);
  p[1] = (unsigned char)((val >> 8) & 0xFF);
  p[2] = (unsigned char)((val >> 16) & 0xFF);
  p[3] = (unsigned char)((val >> 24) & 0xFF);
}

/* A minimal reproduction of the vulnerable path inside coff_arm_relocate_section.
   It fixes up a BL instruction by writing to contents + (rel->r_vaddr - input_section->vma)
   without checking bounds, matching the buggy snippet. */
static void coff_arm_relocate_section(bfd *output_bfd,
                                      bfd *input_bfd,
                                      asection *input_section,
                                      unsigned char *contents,
                                      const reloc_entry *rel)
{
  /* Compute the write location exactly as in the vulnerable code path. */
  unsigned char *where = contents + (size_t)(rel->r_vaddr - input_section->vma);

  /* Dummy values to mirror the original sequence. */
  bfd_signed_vma ret_offset = 0; /* Value isn't relevant for the OOB. */

  /* Read the original instruction value (stubbed to avoid OOB read). */
  uint32_t tmp = bfd_get_32(input_bfd, where);

  /* Vulnerable write: no bounds check before writing 4 bytes at 'where'. */
  bfd_put_32(output_bfd, (bfd_vma)insert_thumb_branch(tmp, ret_offset), where);
}

int main(void) {
  /* Allocate a small section buffer to represent the input section's contents. */
  const size_t section_size = 16;  /* Very small on purpose. */
  unsigned char *contents = (unsigned char *)malloc(section_size);
  if (!contents) {
    perror("malloc");
    return 1;
  }
  memset(contents, 0xAA, section_size);

  /* Set up minimal BFD/section/reloc objects. */
  bfd out_bfd = {0};
  bfd in_bfd = {0};
  asection input_sec;
  memset(&input_sec, 0, sizeof(input_sec));
  input_sec.vma = 0;  /* Typical: contents is indexed from 0 relative to section VMA. */

  reloc_entry rel;
  /* Craft an r_vaddr that is far beyond the end of 'contents'.
     This will make contents + (r_vaddr - vma) point past the buffer,
     triggering an out-of-bounds write. */
  rel.r_vaddr = section_size + 1024;  /* Well beyond the 16-byte buffer. */

  fprintf(stderr, "Section size: %zu, VMA: 0x%llx, r_vaddr: 0x%llx, computed offset: 0x%llx\n",
          section_size,
          (unsigned long long)input_sec.vma,
          (unsigned long long)rel.r_vaddr,
          (unsigned long long)(rel.r_vaddr - input_sec.vma));

  /* Call the vulnerable routine; ASan should flag the OOB write in bfd_put_32. */
  coff_arm_relocate_section(&out_bfd, &in_bfd, &input_sec, contents, &rel);

  /* Cleanup (won't be reached if ASan aborts). */
  free(contents);
  return 0;
}
