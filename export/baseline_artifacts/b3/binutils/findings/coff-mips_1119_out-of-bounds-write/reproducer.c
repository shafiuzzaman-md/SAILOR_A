#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Self-contained reproducer that mirrors the vulnerable logic in
   bfd/coff-mips.c:mips_relocate_section where the address used for
   relocation writes is computed as contents + (r_vaddr - section->vma)
   without validating bounds. */

typedef uint64_t bfd_vma;

typedef struct bfd {
  int dummy;
} bfd;

typedef struct asection asection;
struct asection {
  bfd_vma vma;
  asection *output_section;
  bfd_vma output_offset;
};

typedef struct reloc_howto_type {
  int pc_relative;   /* whether relocation is PC-relative (not used here) */
  unsigned int size; /* size of relocation field in bytes */
} reloc_howto_type;

typedef enum {
  bfd_reloc_ok = 0,
  bfd_reloc_overflow,
  bfd_reloc_notsupported
} bfd_reloc_status_type;

typedef struct internal_reloc {
  bfd_vma r_vaddr;  /* Address within the section to apply relocation. */
  int r_type;       /* Relocation type. */
  int r_symndx;     /* Not used here. */
  int r_extern;     /* Not used here. */
} internal_reloc;

/* Any non-REFHI value will go through the vulnerable path. */
#define MIPS_R_REFHI 0x16

/* Stub for _bfd_relocate_contents: it writes relocation data to the provided
   location without any bounds checks, matching how the real function would
   treat the pointer it is given. */
bfd_reloc_status_type _bfd_relocate_contents(const reloc_howto_type *howto,
                                             bfd *input_bfd,
                                             bfd_vma relocation,
                                             void *location)
{
  (void)input_bfd;
  (void)howto;
  unsigned char *loc = (unsigned char *)location;
  /* Write 4 bytes at the target location to clearly trigger ASan OOB. */
  uint32_t val = (uint32_t)relocation;
  memcpy(loc, &val, sizeof(val));
  return bfd_reloc_ok;
}

/* Minimal reproduction of the vulnerable portion of mips_relocate_section.
   It computes the write address based on r_vaddr and section->vma without
   validating that r_vaddr is within the section's contents buffer. */
static bfd_reloc_status_type mips_relocate_section(bfd *input_bfd,
                                                   asection *input_section,
                                                   unsigned char *contents,
                                                   const reloc_howto_type *howto,
                                                   internal_reloc *int_rel,
                                                   bfd_vma relocation)
{
  /* Skip pc-relative adjustment to keep things simple. */
  if (relocation == 0)
    return bfd_reloc_ok;

  if (int_rel->r_type != MIPS_R_REFHI) {
    unsigned char *ptr = contents + (size_t)(int_rel->r_vaddr - input_section->vma);
    /* Print info to show the computed out-of-bounds pointer. */
    printf("contents=%p, write_ptr=%p, delta=%lld\n",
           (void *)contents, (void *)ptr,
           (long long)(int_rel->r_vaddr - input_section->vma));
    return _bfd_relocate_contents(howto, input_bfd, relocation, ptr);
  }

  return bfd_reloc_ok;
}

int main(void)
{
  /* Allocate a small section buffer so writing just past the end triggers ASan. */
  const size_t section_size = 32;
  unsigned char *contents = (unsigned char *)malloc(section_size);
  if (!contents) {
    perror("malloc");
    return 1;
  }
  memset(contents, 0x41, section_size);

  /* Set up sections. output_section is irrelevant for this path, but we
     provide it to mirror the real structure layout. */
  asection out = { .vma = 0x2000, .output_section = NULL, .output_offset = 0 };
  asection in  = { .vma = 0x1000, .output_section = &out, .output_offset = 0 };

  /* Craft a relocation that targets just past the end of the section buffer:
     r_vaddr - vma = section_size + 1 => write at contents + section_size + 1. */
  internal_reloc rel;
  rel.r_vaddr = in.vma + section_size + 1; /* OOB by 1 byte, 4-byte write => overflow */
  rel.r_type = 1;      /* Not MIPS_R_REFHI so the vulnerable call is used. */
  rel.r_symndx = 0;
  rel.r_extern = 0;

  reloc_howto_type howto;
  howto.pc_relative = 0; /* Keep path simple. */
  howto.size = 4;        /* Typical word-sized relocation. */

  bfd dummy_bfd;

  /* Non-zero relocation so the write happens. */
  bfd_vma relocation = 0x12345678u;

  /* This call reproduces the vulnerable behavior and triggers a heap-buffer-overflow. */
  bfd_reloc_status_type r = mips_relocate_section(&dummy_bfd, &in, contents, &howto, &rel, relocation);
  printf("Relocation status: %d\n", (int)r);

  /* Clean up (may not be reached if ASan aborts on overflow). */
  free(contents);
  return 0;
}
