#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

/* Minimal stand-ins for BFD types used by the vulnerable code. */
typedef uint64_t bfd_vma;

typedef struct bfd {
  int dummy;
} bfd;

typedef struct asection {
  bfd_vma size;
  bfd_vma vma;
} asection;

/* Minimal reloc howto type and helpers. */
typedef struct reloc_howto_struct {
  int size;              /* 2 for 16-bit, 4 for 32-bit writes. */
  bfd_vma dst_mask;
  bfd_vma src_mask;
} reloc_howto_type;

static int bfd_get_reloc_size(const reloc_howto_type *howto) {
  return howto->size;
}

/* Simplified bfd_put_* that just writes to memory (little-endian). */
static void bfd_put_16(bfd *abfd, bfd_vma val, void *location) {
  unsigned char *p = (unsigned char *)location;
  (void)abfd; /* unused in this stub */
  p[0] = (unsigned char)(val & 0xff);
  p[1] = (unsigned char)((val >> 8) & 0xff);
}

static void bfd_put_32(bfd *abfd, bfd_vma val, void *location) {
  unsigned char *p = (unsigned char *)location;
  (void)abfd; /* unused in this stub */
  p[0] = (unsigned char)(val & 0xff);
  p[1] = (unsigned char)((val >> 8) & 0xff);
  p[2] = (unsigned char)((val >> 16) & 0xff);
  p[3] = (unsigned char)((val >> 24) & 0xff);
}

/* This function mirrors the vulnerable write site in bfd/coff-rs6000.c:
 *
 *   if (2 == bfd_get_reloc_size(&howto))
 *     bfd_put_16(input_bfd, value_to_relocate, location);
 *   else
 *     bfd_put_32(input_bfd, value_to_relocate, location);
 *
 * Critically, it only performs the insufficient check: address > input_section->size.
 */
static bool xcoff_ppc_relocate_section(bfd *input_bfd,
                                       asection *input_section,
                                       unsigned char *contents,
                                       bfd_vma address,
                                       reloc_howto_type howto,
                                       bfd_vma value_to_relocate,
                                       bfd_vma relocation) {
  (void)relocation; /* not needed for the reproducer */

  /* Simulate the insufficient bounds check present in the real code. */
  if (address > input_section->size) {
    fprintf(stderr, "Address beyond section size, returning early (no write)\n");
    return false;
  }

  /* Compute the location inside the section contents. */
  unsigned char *location = contents + address;

  /* Minimal value update from the source fragment. Not important for OOB. */
  value_to_relocate = ((value_to_relocate & ~howto.dst_mask)
                       | (((value_to_relocate & howto.src_mask) + relocation) & howto.dst_mask));

  /* Vulnerable write: no check that address + write_size <= input_section->size. */
  if (2 == bfd_get_reloc_size(&howto))
    bfd_put_16(input_bfd, value_to_relocate, location);
  else
    bfd_put_32(input_bfd, value_to_relocate, location);

  return true;
}

int main(void) {
  /* Allocate a small section buffer. */
  const size_t section_size = 16;
  unsigned char *contents = (unsigned char *)malloc(section_size);
  if (!contents) {
    perror("malloc");
    return 1;
  }
  memset(contents, 0xAA, section_size);

  bfd fake_bfd = {0};
  asection input_section;
  input_section.size = section_size;
  input_section.vma = 0;

  /* howto configured for a 16-bit relocation (so bfd_put_16 is used). */
  reloc_howto_type howto;
  howto.size = 2;               /* 2-byte write path */
  howto.dst_mask = 0xFFFF;
  howto.src_mask = 0xFFFF;

  /* Choose an address that is within the section but too close to the end
   * for the 2-byte store, i.e., address == size - 1. The existing check in
   * the vulnerable code only rejects address > size, so this passes and then
   * the 2-byte write overflows by 1 byte.
   */
  bfd_vma address = section_size - 1; /* 15 for 16-byte buffer */

  /* Arbitrary values; not important for triggering the OOB. */
  bfd_vma value_to_relocate = 0xBEEF;
  bfd_vma relocation = 0x1;

  /* This call will perform an out-of-bounds 2-byte store that overlaps the end
   * of the 16-byte buffer by 1 byte, which ASan will report as a heap-buffer-overflow.
   */
  (void)xcoff_ppc_relocate_section(&fake_bfd, &input_section, contents,
                                   address, howto, value_to_relocate, relocation);

  /* Prevent compiler from optimizing out the write. */
  volatile unsigned char sink = contents[section_size - 1];
  (void)sink;

  /* Clean up (ASan will still catch the OOB before or after this). */
  free(contents);
  return 0;
}
